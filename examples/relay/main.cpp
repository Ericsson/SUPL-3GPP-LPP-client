#include <client-io/config.hpp>
#include <client-io/io.hpp>
#include <client-io/types.hpp>
#include <io/input.hpp>
#include <io/registry.hpp>
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

#include <cstdio>
#include <memory>
#include <vector>

LOGLET_MODULE(relay);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(relay)

int main(int argc, char** argv) {
    loglet::initialize();

    args::ArgumentParser parser{"example-relay — raw byte passthrough between streams"};
    stream::setup(parser);
    input::setup(parser);
    output::setup(parser);

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help const&) {
        printf("%s", parser.Help().c_str());
        return 0;
    } catch (args::ParseError const& e) {
        fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }

    StreamsConfig streams_cfg;
    InputsConfig  inputs_cfg;
    OutputsConfig outputs_cfg;
    stream::parse(streams_cfg);
    input::parse(inputs_cfg);
    output::parse(outputs_cfg);

    io::StreamRegistry registry;
    create_streams(streams_cfg, registry);
    create_implicit_streams(inputs_cfg, outputs_cfg, registry);

    // Build outputs
    std::vector<std::unique_ptr<io::Output>> outputs;
    auto                                     add_out = [&](std::unique_ptr<io::Output> o) {
        if (o) outputs.push_back(std::move(o));
    };
    for (auto& cfg : outputs_cfg.stream)
        add_out(create_output(cfg, registry));
    for (auto& cfg : outputs_cfg.stdout_outputs)
        add_out(create_output(cfg, registry));
    for (auto& cfg : outputs_cfg.file)
        add_out(create_output(cfg, registry));
    for (auto& cfg : outputs_cfg.tcp_server)
        add_out(create_output(cfg));
    for (auto& cfg : outputs_cfg.serial)
        add_out(create_output(cfg, registry));
    for (auto& cfg : outputs_cfg.tcp_client)
        add_out(create_output(cfg, registry));
    for (auto& cfg : outputs_cfg.udp_client)
        add_out(create_output(cfg, registry));

    scheduler::Scheduler scheduler;

    // Schedule outputs
    for (auto& out : outputs)
        (void)out->schedule(scheduler);

    // Build inputs, wire raw passthrough callback
    std::vector<std::unique_ptr<io::Input>> inputs;
    auto                                    add_in = [&](std::unique_ptr<io::Input> inp) {
        if (!inp) return;
        inp->callback = [&outputs](io::Input&, uint8_t* data, size_t len) {
            for (auto& out : outputs)
                out->write(data, len);
        };
        (void)inp->schedule(scheduler);
        inputs.push_back(std::move(inp));
    };
    for (auto& cfg : inputs_cfg.stream)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.stdin_inputs)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.file)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.serial)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.tcp_client)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.tcp_server)
        add_in(create_input(cfg, registry));
    for (auto& cfg : inputs_cfg.udp_server)
        add_in(create_input(cfg, registry));

    INFOF("relay: %zu input(s), %zu output(s)", inputs.size(), outputs.size());
    scheduler.execute();
    return 0;
}
