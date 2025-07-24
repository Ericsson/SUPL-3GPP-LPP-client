#include "control.hpp"

#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, ctrl);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, ctrl)

void CtrlPrint::inspect(streamline::System&, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    message->print();
}

void CtrlOutput::inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto payload = message->payload();
    auto data    = reinterpret_cast<uint8_t const*>(payload.data());
    auto size    = payload.size();
    for (auto const& output : mConfig.outputs) {
        if (!output.ctrl_support()) continue;
        if(!output.accept_tag(tag)) {
            XDEBUGF(OUTPUT_PRINT_MODULE, "ctrl: tag %llX not accepted", tag);
            continue;
        }
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "ctrl: %zd bytes", size);
        }
        output.interface->write(data, size);
    }
}

void CtrlEvents::inspect(streamline::System&, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (on_cell_id) {
        auto cell_id = dynamic_cast<format::ctrl::CellId*>(message.get());
        if (cell_id) {
            on_cell_id(*cell_id);
        }
    }

    if (on_identity_imsi) {
        auto identity_imsi = dynamic_cast<format::ctrl::IdentityImsi*>(message.get());
        if (identity_imsi) {
            on_identity_imsi(*identity_imsi);
        }
    }
}
