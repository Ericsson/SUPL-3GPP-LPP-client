#include "ntrip_source.hpp"

#include <loglet/loglet.hpp>

#include <arpa/inet.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

LOGLET_MODULE(ntrip_source);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ntrip_source)

static constexpr double DEG_TO_RAD = M_PI / 180.0;
static constexpr double EARTH_R_M  = 6371000.0;

NtripSource::NtripSource(NtripConfig config, DataCallback on_data,
                         LocationProvider location_provider)
    : mConfig(std::move(config)), mOnData(std::move(on_data)),
      mLocationProvider(std::move(location_provider)), mPollTask(std::chrono::milliseconds(100)),
      mReconnectTask(std::chrono::seconds(mConfig.reconnect_interval_s)),
      mPositionTask(std::chrono::seconds(mConfig.position_interval_s)) {
    mPollTask.callback = [this] {
        poll();
        mPollTask.restart();
    };
    mReconnectTask.callback = [this] {
        connect();
    };
    mPositionTask.callback = [this] {
        send_nmea();
        mPositionTask.restart();
    };
}

NtripSource::~NtripSource() {
    disconnect();
}

bool NtripSource::schedule(scheduler::Scheduler& scheduler) {
    mReconnectTask.set_duration(std::chrono::milliseconds(0));
    mReconnectTask.schedule();
    return true;
}

void NtripSource::connect() {
    disconnect();

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    auto port_str     = std::to_string(mConfig.port);
    if (getaddrinfo(mConfig.host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res) {
        WARNF("NTRIP: failed to resolve %s", mConfig.host.c_str());
        schedule_reconnect();
        return;
    }

    mSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket < 0) {
        freeaddrinfo(res);
        schedule_reconnect();
        return;
    }

    if (::connect(mSocket, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        ::close(mSocket);
        mSocket = -1;
        WARNF("NTRIP: failed to connect to %s:%u", mConfig.host.c_str(), mConfig.port);
        schedule_reconnect();
        return;
    }
    freeaddrinfo(res);
    mHeaderDone = false;

    // Randomise offset per connection
    if (mConfig.position_offset_m > 0) {
        double angle = mOffsetAngle(mRng);
        double dist  = mOffsetDist(mRng) * mConfig.position_offset_m;
        mOffsetLatM  = dist * std::cos(angle);
        mOffsetLonM  = dist * std::sin(angle);
    }

    // HTTP NTRIP request
    std::string req = "GET /" + mConfig.mountpoint + " HTTP/1.0\r\n" + "Host: " + mConfig.host +
                      "\r\n" + "Ntrip-Version: Ntrip/2.0\r\n" +
                      "User-Agent: SUPL-3GPP-LPP-client\r\n";
    if (!mConfig.username.empty()) {
        std::string        creds = mConfig.username + ":" + mConfig.password;
        static char const* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string        enc;
        for (size_t i = 0; i < creds.size(); i += 3) {
            uint32_t v = (uint8_t)creds[i] << 16;
            if (i + 1 < creds.size()) v |= (uint8_t)creds[i + 1] << 8;
            if (i + 2 < creds.size()) v |= (uint8_t)creds[i + 2];
            enc += b64[(v >> 18) & 63];
            enc += b64[(v >> 12) & 63];
            enc += (i + 1 < creds.size()) ? b64[(v >> 6) & 63] : '=';
            enc += (i + 2 < creds.size()) ? b64[v & 63] : '=';
        }
        req += "Authorization: Basic " + enc + "\r\n";
    }
    req += "\r\n";
    ::send(mSocket, req.c_str(), req.size(), 0);

    if (mConfig.position_mode != NtripConfig::PositionMode::None) send_nmea();

    mPollTask.schedule();

    if (mConfig.position_mode == NtripConfig::PositionMode::Internal &&
        mConfig.position_interval_s > 0) {
        mPositionTask.set_duration(std::chrono::seconds(mConfig.position_interval_s));
        mPositionTask.schedule();
    }

    INFOF("NTRIP: connected to %s:%u/%s", mConfig.host.c_str(), mConfig.port,
          mConfig.mountpoint.c_str());
}

void NtripSource::disconnect() {
    mPollTask.cancel();
    mPositionTask.cancel();
    if (mSocket >= 0) {
        ::close(mSocket);
        mSocket = -1;
    }
}

void NtripSource::schedule_reconnect() {
    mReconnectTask.set_duration(std::chrono::seconds(mConfig.reconnect_interval_s));
    mReconnectTask.schedule();
}

void NtripSource::poll() {
    if (mSocket < 0) return;

    uint8_t buf[4096];
    auto    n = ::recv(mSocket, buf, sizeof(buf), MSG_DONTWAIT);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        WARNF("NTRIP: read error, reconnecting");
        disconnect();
        schedule_reconnect();
        return;
    }
    if (n == 0) {
        WARNF("NTRIP: connection closed, reconnecting");
        disconnect();
        schedule_reconnect();
        return;
    }

    size_t offset = 0;
    if (!mHeaderDone) {
        for (size_t i = 0; i + 3 < (size_t)n; i++) {
            if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n') {
                offset      = i + 4;
                mHeaderDone = true;
                break;
            }
        }
        if (!mHeaderDone) return;
    }

    if (offset < (size_t)n && mOnData) mOnData(buf + offset, (size_t)n - offset);
}

void NtripSource::apply_bias(double& lat, double& lon) const {
    if (mConfig.position_round_deg > 0) {
        lat = std::round(lat / mConfig.position_round_deg) * mConfig.position_round_deg;
        lon = std::round(lon / mConfig.position_round_deg) * mConfig.position_round_deg;
    }
    if (mConfig.position_offset_m > 0 && mOffsetLatM != 0.0) {
        lat += mOffsetLatM / EARTH_R_M * (180.0 / M_PI);
        lon += mOffsetLonM / (EARTH_R_M * std::cos(lat * DEG_TO_RAD)) * (180.0 / M_PI);
    }
}

std::string NtripSource::build_gga() const {
    double lat = mConfig.latitude;
    double lon = mConfig.longitude;
    double alt = mConfig.altitude;

    if (mConfig.position_mode == NtripConfig::PositionMode::Internal && mLocationProvider) {
        auto loc = mLocationProvider();
        if (loc.has_value() && loc.const_value().location.has_value()) {
            lat = loc.const_value().location.const_value().latitude();
            lon = loc.const_value().location.const_value().longitude();
        }
    }

    apply_bias(lat, lon);

    char lat_hem   = lat >= 0 ? 'N' : 'S';
    char lon_hem   = lon >= 0 ? 'E' : 'W';
    lat            = std::abs(lat);
    lon            = std::abs(lon);
    double lat_deg = std::floor(lat), lat_min = (lat - lat_deg) * 60.0;
    double lon_deg = std::floor(lon), lon_min = (lon - lon_deg) * 60.0;

    char buf[128];
    snprintf(buf, sizeof(buf),
             "$GPGGA,000000.00,%02.0f%09.6f,%c,%03.0f%09.6f,%c,1,08,1.0,%.1f,M,0.0,M,,", lat_deg,
             lat_min, lat_hem, lon_deg, lon_min, lon_hem, alt);

    uint8_t cs = 0;
    for (int i = 1; buf[i] && buf[i] != '*'; i++)
        cs ^= (uint8_t)buf[i];

    char result[160];
    snprintf(result, sizeof(result), "%s*%02X\r\n", buf, cs);
    return result;
}

void NtripSource::send_nmea() {
    if (mSocket < 0) return;
    auto gga = build_gga();
    ::send(mSocket, gga.c_str(), gga.size(), 0);
}
