#include <client-io/config.hpp>
#include <client-io/io.hpp>
#include <client-io/registry.hpp>
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

static void register_types() {
    io_registry::register_input_type(make_stdin_input_type());
    io_registry::register_input_type(make_file_input_type());
    io_registry::register_input_type(make_serial_input_type());
    io_registry::register_input_type(make_tcp_client_input_type());
    io_registry::register_input_type(make_tcp_server_input_type());
    io_registry::register_input_type(make_udp_server_input_type());
    io_registry::register_input_type(make_stream_ref_input_type());

    io_registry::register_output_type(make_stdout_output_type());
    io_registry::register_output_type(make_file_output_type());
    io_registry::register_output_type(make_tcp_server_output_type());
    io_registry::register_output_type(make_serial_output_type());
    io_registry::register_output_type(make_tcp_client_output_type());
    io_registry::register_output_type(make_udp_client_output_type());
    io_registry::register_output_type(make_stream_ref_output_type());
}

int main(int argc, char** argv) {
    loglet::initialize();
    loglet::set_level(loglet::Level::Info);
    loglet::set_always_flush(true);
    register_types();

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

    std::vector<std::unique_ptr<io::Output>> outputs;
    scheduler::Scheduler                     scheduler;
    scheduler::set_current(&scheduler);

    for (auto& entry : outputs_cfg.outputs) {
        auto out = create_output(entry, registry);
        if (!out) continue;
        INFOF("output: %s", out->name());
        (void)out->schedule(scheduler);
        outputs.push_back(std::move(out));
    }

    std::vector<std::unique_ptr<io::Input>> inputs;
    auto                                    add_in = [&](std::unique_ptr<io::Input> inp) {
        if (!inp) return;
        INFOF("input scheduled");
        inp->callback = [&outputs](io::Input&, uint8_t* data, size_t len) {
            for (auto& out : outputs)
                out->write(data, len);
        };
        (void)inp->schedule(scheduler);
        inputs.push_back(std::move(inp));
    };

    for (auto& entry : inputs_cfg.inputs) {
        add_in(create_input(entry, registry));
    }

    INFOF("relay: %zu input(s), %zu output(s) — running", inputs.size(), outputs.size());
    scheduler.execute();
    return 0;
}
