#pragma once
#include <lpp/message.hpp>

namespace lpp {

struct ProvideCapabilities {
    /// @brief Assistance data support
    struct {
        bool osr;
        bool ssr;
    } assistance_data;

    /// @brief The GNSS systems that are supported
    struct {
        bool gps;
        bool glonass;
        bool galileo;
        bool beidou;
    } gnss;
};

Message create_provide_capabilities(ProvideCapabilities const& capabilities);

}  // namespace lpp
