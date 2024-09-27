#include "ctrl.hpp"

#include <format/ctrl/cid.hpp>
#include <format/ctrl/identity.hpp>

void CtrlPrint::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    message->print();
}

void CtrlOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    auto payload = message->payload();
    auto data    = reinterpret_cast<uint8_t const*>(payload.data());
    auto size    = payload.size();
    for (auto const& output : mOptions.outputs) {
        if ((output.format & OUTPUT_FORMAT_CTRL) != 0) {
            output.interface->write(data, size);
        }
    }
}

void CtrlEvents::inspect(streamline::System&, DataType const& message) NOEXCEPT {
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
