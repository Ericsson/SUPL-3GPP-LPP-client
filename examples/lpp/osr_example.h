#pragma once
#include "options.hpp"

namespace osr_example {

enum class Format {
    RTCM,
    XER,
};

enum class MsmType {
    ANY,
    MSM4,
    MSM5,
    MSM6,
    MSM7,
};

class OsrCommand final : public Command {
public:
    OsrCommand()
        : Command("osr", "Request observation data from a location server"), mFormatArg(nullptr),
          mMsmTypeArg(nullptr) {}

    ~OsrCommand() override {
        delete mFormatArg;
        delete mMsmTypeArg;
    }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
    args::ValueFlag<std::string>* mMsmTypeArg;
};

}  // namespace osr_example
