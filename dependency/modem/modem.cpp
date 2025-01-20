#include "modem.hpp"

#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

#define LOGLET_CURRENT_MODULE "modem"

namespace modem {

Modem::Modem(std::unique_ptr<io::Input> input, std::unique_ptr<io::Output> output) NOEXCEPT
    : mInput(std::move(input)),
      mOutput(std::move(output)) {
    VSCOPE_FUNCTION();

    mInput->callback = [this](io::Input&, uint8_t* buffer, size_t count) {
        VERBOSEF("received %zu bytes", count);
        mParser.append(buffer, static_cast<uint32_t>(count));
        mParser.process();

        while (!mCallbacks.empty()) {
            if (!mParser.has_lines()) {
                return;
            }

            auto state = mCallbacks.front();
            VERBOSEF("wait for response to: \"%s\"", state.request.c_str());

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
                VERBOSEF("unsolicited response: \"%s\"", unsolicited_line.c_str());
            } else {
                auto result = state.callback(mParser);

                // if the message was not fully processed, wait for more data
                if (result != ResponseResult::MissingLines) {
                    mCallbacks.pop();
                }
            }
        }
    };
}

void Modem::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTION();
    mInput->schedule(scheduler);
}

void Modem::cancel() {
    VSCOPE_FUNCTION();
    mInput->cancel();
}

void Modem::enable_echo() {
    VSCOPE_FUNCTION();
    mOutput->write(reinterpret_cast<uint8_t const*>("ATE1\r\n"), 6);
}

void Modem::disable_echo() {
    VSCOPE_FUNCTION();
    mOutput->write(reinterpret_cast<uint8_t const*>("ATE0\r\n"), 6);
}

void Modem::request(std::string const& command, ResponseCallback callback) {
    VSCOPE_FUNCTION();
    if (mOutput) {
        uint8_t buffer[1024];
        auto    length =
            snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer), "%s\r\n", command.c_str());

        mCallbacks.emplace(ResponseState{command, callback});
        mOutput->write(buffer, static_cast<size_t>(length));
    }
}

void Modem::request_no_response(std::string const& command) {
    VSCOPE_FUNCTION();
    if (mOutput) {
        uint8_t buffer[1024];
        auto    length =
            snprintf(reinterpret_cast<char*>(buffer), sizeof(buffer), "%s\r\n", command.c_str());

        mOutput->write(buffer, static_cast<size_t>(length));
    }
}

Modem::ResponseResult Modem::handle_cimi_query(format::at::Parser& parser, Cimi& cimi) {
    VSCOPE_FUNCTION();

    // +CIMI<CR><LF>
    // 123456789012345<CR><LF>
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    if (ok != "OK") {
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
        return ResponseResult::Failure;
    }

    try {
        cimi.imsi = std::stoull(line);
        return ResponseResult::Success;
    } catch (std::exception const& e) {
        WARNF("failed to parse IMSI: \"%s\" %s", line.c_str(), e.what());
    }

    return ResponseResult::Failure;
}

bool Modem::get_cimi(scheduler::Scheduler& scheduler, Cimi& cimi) {
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CIMI", [this, &cimi, &result, &called](format::at::Parser& parser) {
        result = handle_cimi_query(parser, cimi);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

Modem::ResponseResult Modem::handle_creg_test(format::at::Parser& parser,
                                              SupportedCregModes& modes) {
    VSCOPE_FUNCTION();

    // +CREG=?<CR><CR><LF>
    // +CREG: ...<CR><LF>
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto data = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    if (ok != "OK") {
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

    return ResponseResult::Success;
}

bool Modem::list_creg(scheduler::Scheduler& scheduler, SupportedCregModes& modes) {
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CREG=?", [this, &modes, &result, &called](format::at::Parser& parser) {
        result = handle_creg_test(parser, modes);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

void Modem::set_creg(int mode) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "AT+CREG=%d", mode);
    request_no_response(buffer);
}

Modem::ResponseResult Modem::handle_creg_query(format::at::Parser& parser, Creg& reg) {
    VSCOPE_FUNCTION();

    // AT+CREG?<CR><LF>
    // +CREG: <n>,<stat>[,<lac>,<ci>[,<AcT>]]
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    if (ok != "OK") {
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
        return ResponseResult::Failure;
    }

    if (sscanf(line.c_str(), "+CREG: %d,%d,\"%x\",\"%x\",%d", &reg.mode, &reg.status, &reg.lac,
               &reg.ci, &reg.act) != 5) {
        VERBOSEF("failed to parse CREG: \"%s\"", line.c_str());
        return ResponseResult::Failure;
    }

    return ResponseResult::Success;
}

bool Modem::get_creg(scheduler::Scheduler& scheduler, Creg& reg) {
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+CREG?", [this, &reg, &result, &called](format::at::Parser& parser) {
        result = handle_creg_query(parser, reg);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

void Modem::set_cops(int format) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "AT+COPS=3,%d,", format);
    request_no_response(buffer);
}

Modem::ResponseResult Modem::handle_cops_query(format::at::Parser& parser, Cops& cops) {
    VSCOPE_FUNCTION();

    // AT+COPS?<CR><LF>
    // +COPS: <mode>[,<format>,<oper>[,<AcT>]]
    // <CR><LF>
    // OK<CR><LF>

    if (parser.count() != 4) {
        return ResponseResult::MissingLines;
    }

    parser.skip_line();
    auto line = parser.skip_line();
    parser.skip_line();
    auto ok = parser.skip_line();

    if (ok != "OK") {
        return ResponseResult::Failure;
    } else if (line.size() == 0) {
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

        return ResponseResult::Success;
    } else {
        VERBOSEF("unsupported COPS mode/format: %d/%d", cops.mode, cops.format);
        return ResponseResult::Failure;
    }
}

bool Modem::get_cops(scheduler::Scheduler& scheduler, Cops& cops) {
    auto result = ResponseResult::Failure;
    auto called = false;
    request("AT+COPS?", [this, &cops, &result, &called](format::at::Parser& parser) {
        result = handle_cops_query(parser, cops);
        if (result != ResponseResult::MissingLines) {
            called = true;
        }
        return result;
    });

    scheduler.execute_while([&]() {
        return !called;
    });
    return result == ResponseResult::Success;
}

}  // namespace modem
