#include <format/lpp/uper_parser.hpp>
#include <format/tbin/reader.hpp>
#include <format/tbin/writer.hpp>
#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/utc.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <LPP-Message.h>
EXTERNAL_WARNINGS_POP

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int64_t utc_to_us(int year, int month, int day, int hour, int min, int sec, int nano) {
    struct tm t{};
    t.tm_year  = year - 1900;
    t.tm_mon   = month - 1;
    t.tm_mday  = day;
    t.tm_hour  = hour;
    t.tm_min   = min;
    t.tm_sec   = sec;
    auto epoch = timegm(&t);
    return static_cast<int64_t>(epoch) * 1000000LL + nano / 1000;
}

static int64_t utc_us(ts::Utc const& utc) {
    return static_cast<int64_t>(utc.timestamp().seconds()) * 1000000LL +
           static_cast<int64_t>(utc.timestamp().fraction() * 1e6);
}

// --- UBX Parser ---
struct UbxFrame {
    uint8_t  cls;
    uint8_t  id;
    uint16_t length;
    size_t   offset;  // offset in file of sync byte
};

static bool find_ubx_frame(uint8_t const* buf, size_t len, size_t& pos, UbxFrame& frame) {
    while (pos + 8 <= len) {
        if (buf[pos] == 0xB5 && buf[pos + 1] == 0x62) {
            frame.cls    = buf[pos + 2];
            frame.id     = buf[pos + 3];
            frame.length = buf[pos + 4] | (buf[pos + 5] << 8);
            frame.offset = pos;
            if (pos + 6 + frame.length + 2 <= len) {
                return true;
            }
        }
        pos++;
    }
    return false;
}

static int64_t extract_ubx_timestamp(uint8_t const* payload, UbxFrame const& frame) {
    // NAV-PVT (0x01 0x07)
    if (frame.cls == 0x01 && frame.id == 0x07 && frame.length >= 20) {
        uint16_t year  = payload[4] | (payload[5] << 8);
        uint8_t  month = payload[6];
        uint8_t  day   = payload[7];
        uint8_t  hour  = payload[8];
        uint8_t  min   = payload[9];
        uint8_t  sec   = payload[10];
        int32_t  nano;
        memcpy(&nano, payload + 16, 4);
        uint8_t valid = payload[11];
        if ((valid & 0x03) == 0x03 && year >= 2000) {
            return utc_to_us(year, month, day, hour, min, sec, nano);
        }
    }
    return -1;
}

// --- RTCM Parser ---
static bool find_rtcm_frame(uint8_t const* buf, size_t len, size_t& pos, size_t& frame_len,
                            uint16_t& msg_type) {
    while (pos + 6 <= len) {
        if (buf[pos] == 0xD3 && (buf[pos + 1] & 0xFC) == 0) {
            uint16_t payload_len = ((buf[pos + 1] & 0x03) << 8) | buf[pos + 2];
            frame_len            = 3 + payload_len + 3;
            if (pos + frame_len <= len) {
                msg_type = (buf[pos + 3] << 4) | (buf[pos + 4] >> 4);
                return true;
            }
        }
        pos++;
    }
    return false;
}

static int64_t extract_rtcm_timestamp(uint8_t const* buf, size_t frame_len, uint16_t msg_type,
                                      int gps_week, ts::Gps const& reference) {
    if (frame_len < 10) return -1;
    uint8_t const* p = buf + 3;

    // All MSM: bits 0-11 = MT, 12-23 = ref station, 24-53 = 30-bit epoch
    auto epoch30 = [&]() -> int64_t {
        uint64_t val = (static_cast<uint64_t>(p[3]) << 22) | (static_cast<uint64_t>(p[4]) << 14) |
                       (static_cast<uint64_t>(p[5]) << 6) | (static_cast<uint64_t>(p[6]) >> 2);
        return static_cast<int64_t>(val & 0x3FFFFFFF);
    };

    ts::Utc utc;
    if (msg_type >= 1071 && msg_type <= 1077) {
        // GPS: 30-bit TOW in ms
        utc = ts::Utc(
            ts::Gps::from_week_tow(gps_week, epoch30() / 1000LL, (epoch30() % 1000) / 1000.0));
    } else if (msg_type >= 1081 && msg_type <= 1087) {
        // GLONASS: 3-bit day-of-week + 27-bit ms-of-day
        int64_t e   = epoch30();
        int64_t day = (e >> 27) & 0x7;
        double  tod = (e & 0x7FFFFFF) / 1000.0;
        utc         = ts::Utc(ts::Glo::from_period_day_tod(day, tod, ts::Glo(reference)));
    } else if (msg_type >= 1091 && msg_type <= 1097) {
        // Galileo GST: 30-bit TOW in ms (GST week = GPS week - 1024)
        int64_t gst_week = gps_week - 1024;
        utc              = ts::Utc(
            ts::Gst::from_week_tow(gst_week, epoch30() / 1000LL, (epoch30() % 1000) / 1000.0));
    } else if (msg_type >= 1101 && msg_type <= 1107) {
        // SBAS: GPS time
        utc = ts::Utc(
            ts::Gps::from_week_tow(gps_week, epoch30() / 1000LL, (epoch30() % 1000) / 1000.0));
    } else if (msg_type >= 1111 && msg_type <= 1117) {
        // QZSS: GPS time
        utc = ts::Utc(
            ts::Gps::from_week_tow(gps_week, epoch30() / 1000LL, (epoch30() % 1000) / 1000.0));
    } else if (msg_type >= 1121 && msg_type <= 1127) {
        // BeiDou: 30-bit TOW in ms (BDT week = GPS week - 1356)
        int64_t bdt_week = gps_week - 1356;
        int64_t e        = epoch30();
        utc = ts::Utc(ts::Bdt::from_week_tow(bdt_week, e / 1000LL, (e % 1000) / 1000.0));
    } else {
        return -1;
    }
    return utc_us(utc);
}

// --- LPP UPER Parser ---
// LPP UPER messages are length-prefixed in our capture format (one TCP message = one LPP PDU)
// No inherent timestamp — we assign sequential timestamps

static void parse_ubx(std::string const& input, std::string const& output) {
    FILE* f = fopen(input.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", input.c_str());
        return;
    }
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> buf(size);
    fread(buf.data(), 1, size, f);
    fclose(f);

    format::tbin::Writer writer;
    if (!writer.open(output, "ubx")) {
        fprintf(stderr, "error: cannot open output %s\n", output.c_str());
        return;
    }

    int64_t              current_ts = 0;
    size_t               pos        = 0;
    uint64_t             count      = 0;
    std::vector<uint8_t> pending;
    int64_t              pending_ts = -1;

    while (pos < buf.size()) {
        UbxFrame frame;
        size_t   search_pos = pos;
        if (!find_ubx_frame(buf.data(), buf.size(), search_pos, frame)) break;

        size_t         frame_size = 6 + frame.length + 2;
        uint8_t const* payload    = buf.data() + search_pos + 6;
        int64_t        ts         = extract_ubx_timestamp(payload, frame);

        if (ts > 0) {
            // Flush any pending messages with previous timestamp
            if (!pending.empty() && pending_ts > 0) {
                writer.write(pending_ts, pending.data(), static_cast<uint32_t>(pending.size()));
                count++;
                pending.clear();
            }
            current_ts = ts;
        }

        // Write this frame
        if (current_ts > 0) {
            writer.write(current_ts, buf.data() + search_pos, static_cast<uint32_t>(frame_size));
            count++;
        } else {
            // Buffer until we get a timestamp
            pending.insert(pending.end(), buf.data() + search_pos,
                           buf.data() + search_pos + frame_size);
            pending_ts = current_ts;
        }

        pos = search_pos + frame_size;
    }

    writer.close();
    fprintf(stderr, "ubx: %lu messages written\n", count);
}

static void parse_rtcm(std::string const& input, std::string const& output, int gps_week) {
    FILE* f = fopen(input.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", input.c_str());
        return;
    }
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> buf(size);
    fread(buf.data(), 1, size, f);
    fclose(f);

    format::tbin::Writer writer;
    if (!writer.open(output, "rtcm")) {
        fprintf(stderr, "error: cannot open output %s\n", output.c_str());
        return;
    }

    // GPS TOW rollover detection — GPS MSM only (1071-1077) for reliable reference
    int64_t  current_ts  = 0;
    int64_t  prev_tow_ms = -1;
    int      week        = gps_week;
    ts::Gps  reference   = ts::Gps::from_week_tow(gps_week, 0, 0);
    size_t   pos         = 0;
    uint64_t count       = 0;

    while (pos < buf.size()) {
        size_t   frame_len;
        uint16_t msg_type;
        size_t   search_pos = pos;
        if (!find_rtcm_frame(buf.data(), buf.size(), search_pos, frame_len, msg_type)) break;

        if (msg_type >= 1071 && msg_type <= 1077 && frame_len >= 10) {
            uint8_t const* p      = buf.data() + search_pos + 3;
            int64_t        tow_ms = (static_cast<int64_t>(p[3]) << 22) |
                             (static_cast<int64_t>(p[4]) << 14) |
                             (static_cast<int64_t>(p[5]) << 6) | (static_cast<int64_t>(p[6]) >> 2);
            tow_ms &= 0x3FFFFFFF;
            if (prev_tow_ms >= 0 && prev_tow_ms - tow_ms > 302400000LL) {
                week++;
                reference = ts::Gps::from_week_tow(week, 0, 0);
            }
            prev_tow_ms = tow_ms;
        }

        int64_t ts =
            extract_rtcm_timestamp(buf.data() + search_pos, frame_len, msg_type, week, reference);
        if (ts > 0) current_ts = ts;

        if (current_ts > 0) {
            writer.write(current_ts, buf.data() + search_pos, static_cast<uint32_t>(frame_len));
            count++;
        }

        pos = search_pos + frame_len;
    }

    writer.close();
    fprintf(stderr, "rtcm: %lu messages written (gps_week=%d)\n", count, gps_week);
}

static void parse_lpp_uper(std::string const& input, std::string const& output,
                           int64_t start_time_us, int interval_ms) {
    FILE* f = fopen(input.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", input.c_str());
        return;
    }
    fseek(f, 0, SEEK_END);
    auto file_size = static_cast<size_t>(ftell(f));
    fseek(f, 0, SEEK_SET);

    format::tbin::Writer writer;
    if (!writer.open(output, "lpp-uper")) {
        fprintf(stderr, "error: cannot open output %s\n", output.c_str());
        fclose(f);
        return;
    }

    format::lpp::UperParser parser;

    int64_t  ts         = start_time_us;
    int64_t  interval   = interval_ms * 1000LL;
    uint64_t count      = 0;
    size_t   total_read = 0;

    // Track raw bytes to extract message data
    std::vector<uint8_t> raw_buffer;
    size_t               raw_consumed = 0;

    uint8_t chunk[4096];
    while (total_read < file_size || parser.buffer_length() >= 6) {
        // Feed more data if parser needs it and file has more
        if (parser.buffer_length() < 8192 && total_read < file_size) {
            size_t to_read = std::min(sizeof(chunk), file_size - total_read);
            size_t got     = fread(chunk, 1, to_read, f);
            if (got > 0) {
                parser.append(chunk, got);
                raw_buffer.insert(raw_buffer.end(), chunk, chunk + got);
                total_read += got;
            }
        }

        if (parser.buffer_length() < 6) break;

        size_t before  = parser.buffer_length();
        auto*  message = parser.try_parse();
        if (!message) {
            if (parser.buffer_length() == before) {
                if (total_read >= file_size) break;
                continue;
            }
            size_t skipped = before - parser.buffer_length();
            raw_consumed += skipped;
            continue;
        }

        size_t consumed = before - parser.buffer_length();

        // Extract raw bytes for this message
        std::vector<uint8_t> msg_bytes(raw_buffer.begin() + static_cast<long>(raw_consumed),
                                       raw_buffer.begin() +
                                           static_cast<long>(raw_consumed + consumed));
        raw_consumed += consumed;

        writer.write(ts, msg_bytes.data(), static_cast<uint32_t>(msg_bytes.size()));
        count++;
        ts += interval;

        ASN_STRUCT_FREE(asn_DEF_LPP_Message, message);

        // Compact raw_buffer to avoid unbounded growth
        if (raw_consumed > 1024 * 1024) {
            raw_buffer.erase(raw_buffer.begin(),
                             raw_buffer.begin() + static_cast<long>(raw_consumed));
            raw_consumed = 0;
        }
    }

    fclose(f);
    writer.close();
    fprintf(stderr, "lpp-uper: %lu messages written\n", count);
}

static void parse_rinex_nav(std::string const& input, std::string const& output) {
    FILE* f = fopen(input.c_str(), "r");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", input.c_str());
        return;
    }

    format::tbin::Writer writer;
    if (!writer.open(output, "nav")) {
        fprintf(stderr, "error: cannot open output %s\n", output.c_str());
        fclose(f);
        return;
    }

    // Skip header
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "END OF HEADER")) break;
    }

    // Parse records: first line has SV + Toc, then 7 continuation lines (GPS/GAL/BDS)
    // GLONASS has 3 continuation lines, but we'll just count non-indented lines as record starts
    uint64_t    count = 0;
    std::string record;
    int64_t     record_ts = 0;

    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len == 0) continue;

        // New record starts with a non-space character (satellite ID)
        bool is_record_start = (line[0] != ' ' && line[0] != '\n' && line[0] != '\r');

        if (is_record_start) {
            // Flush previous record
            if (!record.empty() && record_ts > 0) {
                writer.write(record_ts, reinterpret_cast<uint8_t const*>(record.data()),
                             static_cast<uint32_t>(record.size()));
                count++;
            }
            record.clear();

            // Parse Toc: "G16 2026 06 04 08 00 00 ..."
            int year, month, day, hour, min, sec;
            if (sscanf(line + 3, " %d %d %d %d %d %d", &year, &month, &day, &hour, &min, &sec) ==
                6) {
                record_ts = utc_to_us(year, month, day, hour, min, sec, 0);
            }
        }

        record += line;
    }

    // Flush last record
    if (!record.empty() && record_ts > 0) {
        writer.write(record_ts, reinterpret_cast<uint8_t const*>(record.data()),
                     static_cast<uint32_t>(record.size()));
        count++;
    }

    fclose(f);
    writer.close();
    fprintf(stderr, "rinex-nav: %lu records written\n", count);
}

static void print_usage() {
    fprintf(stderr,
            "Usage: tbin-parse --format <fmt> --input <file> --output <file> [options]\n"
            "\n"
            "Formats: ubx, rtcm, lpp-uper, rinex-nav\n"
            "\n"
            "Options:\n"
            "  --gps-week <week>                    GPS week for RTCM timestamp resolution\n"
            "  --start-time <ISO8601>               Start time for formats without timestamps\n"
            "  --interval <ms>                      Interval between messages (default: 1000)\n");
}

int main(int argc, char* argv[]) {
    std::string format, input, output;
    int         gps_week      = 0;
    int64_t     start_time_us = 0;
    int         interval_ms   = 1000;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "--format") && i + 1 < argc)
            format = argv[++i];
        else if ((arg == "--input" || arg == "-i") && i + 1 < argc)
            input = argv[++i];
        else if ((arg == "--output" || arg == "-o") && i + 1 < argc)
            output = argv[++i];
        else if (arg == "--gps-week" && i + 1 < argc)
            gps_week = atoi(argv[++i]);
        else if (arg == "--start-time" && i + 1 < argc) {
            // Parse ISO8601: YYYY-MM-DDTHH:MM:SS
            std::string ts_str = argv[++i];
            struct tm   t{};
            if (sscanf(ts_str.c_str(), "%d-%d-%dT%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday,
                       &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
                t.tm_year -= 1900;
                t.tm_mon -= 1;
                start_time_us = static_cast<int64_t>(timegm(&t)) * 1000000LL;
            }
        } else if (arg == "--interval" && i + 1 < argc)
            interval_ms = atoi(argv[++i]);
        else if (arg == "--info")
            format = "info";
        else if (arg == "--help" || arg == "-h") {
            print_usage();
            return 0;
        }
    }

    if (format == "info") {
        // tbin-parse --info --input file.tbin
        if (input.empty()) {
            fprintf(stderr, "error: --input required for --info\n");
            return 1;
        }
        format::tbin::Reader reader;
        if (!reader.open(input)) {
            fprintf(stderr, "error: cannot open %s\n", input.c_str());
            return 1;
        }
        printf("File:     %s\n", input.c_str());
        printf("Stream:   %s\n", reader.stream_id().c_str());

        format::tbin::Message msg;
        uint64_t              count = 0, ubx = 0, rtcm = 0, lpp = 0, nav = 0, other = 0;
        int64_t               first_ts = 0, last_ts = 0;
        size_t                total_bytes = 0;
        while (reader.next(msg)) {
            if (count == 0) first_ts = msg.timestamp_us;
            last_ts = msg.timestamp_us;
            total_bytes += msg.data.size();
            count++;
            if (msg.data.size() >= 2 && msg.data[0] == 0xB5 && msg.data[1] == 0x62)
                ubx++;
            else if (msg.data.size() >= 1 && msg.data[0] == 0xD3)
                rtcm++;
            else if (msg.data.size() >= 1 && msg.data[0] >= 'A' && msg.data[0] <= 'Z')
                nav++;
            else
                lpp++;
        }
        double duration = (last_ts - first_ts) / 1e6;
        auto   t1       = static_cast<time_t>(first_ts / 1000000);
        auto   t2       = static_cast<time_t>(last_ts / 1000000);
        char   buf1[32], buf2[32];
        strftime(buf1, sizeof(buf1), "%Y-%m-%dT%H:%M:%S", gmtime(&t1));
        strftime(buf2, sizeof(buf2), "%Y-%m-%dT%H:%M:%S", gmtime(&t2));
        printf("Messages: %lu\n", count);
        printf("Bytes:    %zu (payload)\n", total_bytes);
        printf("First:    %s UTC\n", buf1);
        printf("Last:     %s UTC\n", buf2);
        printf("Duration: %.1fs (%.1f min)\n", duration, duration / 60.0);
        printf("Types:    UBX=%lu RTCM=%lu LPP=%lu NAV=%lu\n", ubx, rtcm, lpp, nav);
        if (duration > 0) printf("Rate:     %.1f msg/s\n", count / duration);
        return 0;
    }

    if (format.empty() || input.empty() || output.empty()) {
        print_usage();
        return 1;
    }

    if (format == "ubx") {
        parse_ubx(input, output);
    } else if (format == "rtcm") {
        if (gps_week == 0) {
            fprintf(stderr, "error: --gps-week required for RTCM format\n");
            return 1;
        }
        parse_rtcm(input, output, gps_week);
    } else if (format == "lpp-uper") {
        parse_lpp_uper(input, output, start_time_us, interval_ms);
    } else if (format == "rinex-nav") {
        parse_rinex_nav(input, output);
    } else {
        fprintf(stderr, "error: unsupported format '%s'\n", format.c_str());
        return 1;
    }

    return 0;
}
