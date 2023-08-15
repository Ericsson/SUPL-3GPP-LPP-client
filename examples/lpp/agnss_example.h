#pragma once
#include "options.hpp"

namespace agnss_example {

enum class Format {
    XER,
};

class AgnssCommand final : public Command {
public:
    AgnssCommand()
        : Command("agnss", "Request Assisted GNSS data from the location server"),
          mFormatArg(nullptr) {}

    ~AgnssCommand() override { delete mFormatArg; }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
};

}  // namespace agnss_example
