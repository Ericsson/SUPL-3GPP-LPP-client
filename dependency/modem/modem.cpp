#include "modem.hpp"

#include <cctype>
#include <cinttypes>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE(modem);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(modem)

namespace modem {

#if !defined(DISABLE_VERBOSE)
static void hexdump(uint8_t const* data, size_t size) {
    if (!loglet::is_module_level_enabled(LOGLET_CURRENT_MODULE, loglet::Level::Verbose)) {
        return;
    }

    char print_buffer[512];
    for (size_t i = 0; i < size;) {
        size_t print_count = 0;
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                print_count += static_cast<size_t>(snprintf(print_buffer + print_count,
                                                            sizeof(print_buffer) - print_count,
                                                            "%02X ", data[i + j]));
            } else {
                print_count += static_cast<size_t>(snprintf(
                    print_buffer + print_count, sizeof(print_buffer) - print_count, "   "));
            }
        }
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                print_count += static_cast<size_t>(
                    snprintf(print_buffer + print_count, sizeof(print_buffer) - print_count, "%c",
                             isprint(data[i + j]) ? data[i + j] : '.'));
            }
        }
        TRACEF("%s", print_buffer);
        i += 16;
    }
}
#endif

Modem::Modem(std::unique_ptr<io::Input> input, std::unique_ptr<io::Output> output) NOEXCEPT
    : mInput(std::move(input)),
      mOutput(std::move(output)) {
    FUNCTION_SCOPE();

    mInput->callback = [this](io::Input&, uint8_t* buffer, size_t count) {
        DEBUGF("received %zu bytes", count);
#if !defined(DISABLE_VERBOSE)
        hexdump(buffer, count);
#endif
        mParser.append(buffer, static_cast<uint32_t>(count));
        mParser.process();

        while (!mCallbacks.empty()) {
            if (!mParser.has_lines()) {
                VERBOSEF("waiting for more data");
                return;
            }

            auto state = mCallbacks.front();
            DEBUGF("checking response for: \"%s\"", state.request.c_str());

            auto is_unsolicted = true;
            auto line          = mParser.peek_line();

            if (line.size() == state.request.size() + 1) {
                if (line[line.size() - 1] == '\r' &&
                    line.substr(0, state.request.size()) == state.request) {
                    is_unsolicted = false;
                }
            }

            if (is_unsolicted) {
                auto unsolicited_line = mParser.skip_line();
                DEBUGF("unsolicited response: \"%s\"", unsolicited_line.c_str());
            } else {
                auto result = state.callback(mParser);

                if (result != ResponseResult::MissingLines) {
                    mCallbacks.pop();
                    VERBOSEF("response processed");
                } else {
                    VERBOSEF("waiting for more lines");
                }
            }
        }
    };
}

bool Modem::schedule(scheduler::Scheduler& scheduler) {
    FUNCTION_SCOPE();
    return mInput->schedule(scheduler);
}

bool Modem::cancel() {
    FUNCTION_SCOPE();
    return mInput->cancel();
}

void Modem::enable_echo() {
    FUNCTION_SCOPE();
    DEBUGF("sending: ATE1");
#if !defined(DISABLE_VERBOSE)
    hexdump(reinterpret_cast<uint8_t const*>("ATE1\r\n"), 6);
#endif
    mOutput->write(reinterpret_cast<uint8_t const*>("ATE1\r\n"), 6);
}

void Modem::disable_echo() {
    FUNCTION_SCOPE();
    DEBUGF("sending: ATE0");
#if !defined(DISABLE_VERBOSE)
    hexdump(reinterpret_cast<uint8_t const*>("ATE0\r\n"), 6);
#endif
    mOutput->write(reinterpret_cast<uint8_t const*>("ATE0\r\n"), 6);
}

void Modem::request(std::string const& command, ResponseCallback callback) {
    FUNCTION_SCOPE();
    if (!mOutput) {
        ERRORF("output is null, cannot send request: %s", command.c_str());
        return;
    }

    uint8_t buffer[1024];
    auto    length =
        snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer), "%s\r\n", command.c_str());

    DEBUGF("sending: %s", command.c_str());
#if !defined(DISABLE_VERBOSE)
    hexdump(buffer, static_cast<size_t>(length));
#endif
    mCallbacks.emplace(ResponseState{command, callback});
    mOutput->write(buffer, static_cast<size_t>(length));
}

void Modem::request_no_response(std::string const& command) {
    FUNCTION_SCOPE();
    if (!mOutput) {
        ERRORF("output is null, cannot send request: %s", command.c_str());
        return;
    }

    uint8_t buffer[1024];
    auto    length =
        snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer), "%s\r\n", command.c_str());

    DEBUGF("sending (no response): %s", command.c_str());
#if !defined(DISABLE_VERBOSE)
    hexdump(buffer, static_cast<size_t>(length));
#endif
    mOutput->write(buffer, static_cast<size_t>(length));
}

Modem::ResponseResult Modem::handle_cimi_query(format::at::Parser& parser, Cimi& cimi) {
    FUNCTION_SCOPE();

    // +CIMI<CR><LF>
    // 123456789012345<CR><LF>
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        VERBOSEF("waiting for 4 lines, have %zu", parser.count());
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    VERBOSEF("CIMI response: line=\"%s\" ok=\"%s\"", line.c_str(), ok.c_str());

    if (ok != "OK") {
        VERBOSEF("response not OK: \"%s\"", ok.c_str());
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
        VERBOSEF("IMSI line is empty");
        return ResponseResult::Failure;
    }

    try {
        cimi.imsi = std::stoull(line);
        DEBUGF("parsed IMSI: %" PRIu64, cimi.imsi);
        return ResponseResult::Success;
    } catch (std::exception const& e) {
        WARNF("failed to parse IMSI: \"%s\" %s", line.c_str(), e.what());
    }

    return ResponseResult::Failure;
}

bool Modem::get_cimi(scheduler::Scheduler& scheduler, Cimi& cimi) {
    FUNCTION_SCOPE();
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CIMI", [this, &cimi, &result, &called](format::at::Parser& parser) {
        result = handle_cimi_query(parser, cimi);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    DEBUGF("waiting for CIMI response");
    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

Modem::ResponseResult Modem::handle_creg_test(format::at::Parser& parser,
                                              SupportedCregModes& modes) {
    FUNCTION_SCOPE();

    // +CREG=?<CR><CR><LF>
    // +CREG: ...<CR><LF>
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        VERBOSEF("waiting for 4 lines, have %zu", parser.count());
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto data = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    VERBOSEF("CREG test response: data=\"%s\" ok=\"%s\"", data.c_str(), ok.c_str());

    if (ok != "OK") {
        VERBOSEF("response not OK: \"%s\"", ok.c_str());
        return ResponseResult::Failure;
    }

    modes = {};
    if (data.find("+CREG: (0)") != std::string::npos) {
        modes.mode_0 = true;
    } else if (data.find("+CREG: (0-1)") != std::string::npos) {
        modes.mode_0 = true;
        modes.mode_1 = true;
    } else if (data.find("+CREG: (0-2)") != std::string::npos) {
        modes.mode_0 = true;
        modes.mode_1 = true;
        modes.mode_2 = true;
    } else {
        WARNF("unknown CREG mode: \"%s\"", data.c_str());
    }

    DEBUGF("CREG modes: mode_0=%d mode_1=%d mode_2=%d", modes.mode_0, modes.mode_1, modes.mode_2);
    return ResponseResult::Success;
}

bool Modem::list_creg(scheduler::Scheduler& scheduler, SupportedCregModes& modes) {
    FUNCTION_SCOPE();
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CREG=?", [this, &modes, &result, &called](format::at::Parser& parser) {
        result = handle_creg_test(parser, modes);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    DEBUGF("waiting for CREG test response");
    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

void Modem::set_creg(int mode) {
    FUNCTION_SCOPE();
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "AT+CREG=%d", mode);
    DEBUGF("setting CREG mode to %d", mode);
    request_no_response(buffer);
}

Modem::ResponseResult Modem::handle_creg_query(format::at::Parser& parser, Creg& reg) {
    FUNCTION_SCOPE();

    // AT+CREG?<CR><LF>
    // +CREG: <n>,<stat>[,<lac>,<ci>[,<AcT>]]
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        VERBOSEF("waiting for 4 lines, have %zu", parser.count());
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    VERBOSEF("CREG query response: line=\"%s\" ok=\"%s\"", line.c_str(), ok.c_str());

    if (ok != "OK") {
        VERBOSEF("response not OK: \"%s\"", ok.c_str());
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
        VERBOSEF("CREG line is empty");
        return ResponseResult::Failure;
    }

    if (sscanf(line.c_str(), "+CREG: %d,%d,\"%x\",\"%x\",%d", &reg.mode, &reg.status, &reg.lac,
               &reg.ci, &reg.act) != 5) {
        VERBOSEF("failed to parse CREG: \"%s\"", line.c_str());
        return ResponseResult::Failure;
    }

    DEBUGF("parsed CREG: mode=%d status=%d lac=%u ci=%u act=%d", reg.mode, reg.status, reg.lac,
           reg.ci, reg.act);
    return ResponseResult::Success;
}

bool Modem::get_creg(scheduler::Scheduler& scheduler, Creg& reg) {
    FUNCTION_SCOPE();
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CREG?", [this, &reg, &result, &called](format::at::Parser& parser) {
        result = handle_creg_query(parser, reg);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    DEBUGF("waiting for CREG query response");
    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

void Modem::set_cops(int format) {
    FUNCTION_SCOPE();
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "AT+COPS=3,%d,", format);
    DEBUGF("setting COPS format to %d", format);
    request_no_response(buffer);
}

Modem::ResponseResult Modem::handle_cops_query(format::at::Parser& parser, Cops& cops) {
    FUNCTION_SCOPE();

    // AT+COPS?<CR><LF>
    // +COPS: <mode>[,<format>,<oper>[,<AcT>]]
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        VERBOSEF("waiting for 4 lines, have %zu", parser.count());
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    VERBOSEF("COPS query response: line=\"%s\" ok=\"%s\"", line.c_str(), ok.c_str());

    if (ok != "OK") {
        VERBOSEF("response not OK: \"%s\"", ok.c_str());
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
        VERBOSEF("COPS line is empty");
        return ResponseResult::Failure;
    }

    if (sscanf(line.c_str(), "+COPS: %d,%d", &cops.mode, &cops.format) != 2) {
        VERBOSEF("failed to parse COPS: \"%s\"", line.c_str());
        return ResponseResult::Failure;
    }

    if (cops.mode == 0 && cops.format == 2) {
        if (sscanf(line.c_str(), "+COPS: %d,%d,\"%03d%d\",%d", &cops.mode, &cops.format, &cops.mcc,
                   &cops.mnc, &cops.act) != 5) {
            VERBOSEF("failed to parse COPS: \"%s\"", line.c_str());
            return ResponseResult::Failure;
        }

        DEBUGF("parsed COPS: mode=%d format=%d mcc=%d mnc=%d act=%d", cops.mode, cops.format,
               cops.mcc, cops.mnc, cops.act);
        return ResponseResult::Success;
    } else {
        VERBOSEF("unsupported COPS mode/format: %d/%d", cops.mode, cops.format);
        return ResponseResult::Failure;
    }
}

bool Modem::get_cops(scheduler::Scheduler& scheduler, Cops& cops) {
    FUNCTION_SCOPE();
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+COPS?", [this, &cops, &result, &called](format::at::Parser& parser) {
        result = handle_cops_query(parser, cops);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    DEBUGF("waiting for COPS query response");
    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

}  // namespace modem
