#pragma once
#include <core/core.hpp>

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <format/at/parser.hpp>
#include <io/input.hpp>
#include <io/output.hpp>

namespace modem {

struct Response {
    std::vector<std::string> lines;
    bool                     success;
};

struct SupportedCregModes {
    bool mode_0;
    bool mode_1;
    bool mode_2;
    bool mode_3;
};

struct Cimi {
    uint64_t imsi;
};

struct Creg {
    int      mode;
    int      status;
    uint32_t lac;
    uint32_t ci;
    int      act;

    bool is_gsm() const { return act >= 0 && act < 7; }
    bool is_lte() const { return act >= 7 && act < 11; }
};

struct Cops {
    int mode;
    int format;
    int mcc;
    int mnc;
    int act;
};

struct SupportedCopsModes {};

class Modem {
public:
    EXPLICIT Modem(std::unique_ptr<io::Input> input, std::unique_ptr<io::Output> output) NOEXCEPT;

    void schedule(scheduler::Scheduler& scheduler);
    void cancel();

    /// Enable echo-ing of commands.
    void enable_echo();

    /// Request CIMI (International Mobile Subscriber Identity) of the SIM card.
    bool get_cimi(scheduler::Scheduler& scheduler, Cimi& cimi);

    /// Get supported CREG modes.
    bool list_creg(scheduler::Scheduler& scheduler, SupportedCregModes& modes);
    /// Set CREG mode.
    void set_creg(int mode);
    /// Get CREG status.
    bool get_creg(scheduler::Scheduler& scheduler, Creg& reg);

    /// Set COPS format.
    void set_cops(int format);
    /// Get COPS status.
    bool get_cops(scheduler::Scheduler& scheduler, Cops& cops);

private:
    enum ResponseResult {
        Success,
        Failure,
        MissingLines,
    };

    using ResponseCallback = std::function<ResponseResult(format::at::Parser&)>;

    struct ResponseState {
        std::string      request;
        ResponseCallback callback;
    };

    void request(std::string const& command, ResponseCallback callback);
    void request_no_response(std::string const& command);

    ResponseResult handle_cimi_query(format::at::Parser& parser, Cimi& cimi);
    ResponseResult handle_creg_test(format::at::Parser& parser, SupportedCregModes& modes);
    ResponseResult handle_creg_query(format::at::Parser& parser, Creg& reg);
    ResponseResult handle_cops_query(format::at::Parser& parser, Cops& cops);

    std::unique_ptr<io::Input>  mInput;
    std::unique_ptr<io::Output> mOutput;
    format::at::Parser          mParser;
    std::queue<ResponseState>   mCallbacks;
};

}  // namespace modem
