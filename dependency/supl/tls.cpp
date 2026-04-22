#include "supl/tls.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(supl, tls);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, tls)

namespace supl {

#if defined(USE_OPENSSL)
std::unique_ptr<TlsBackend> create_tls_backend_openssl(TlsConfig const& config);
#endif

std::unique_ptr<TlsBackend> create_tls_backend(TlsConfig const& config) {
    if (!config.enabled) return nullptr;

#if defined(USE_OPENSSL)
    return create_tls_backend_openssl(config);
#else
    ERRORF("TLS requested but no TLS backend is compiled in (enable USE_OPENSSL)");
    return nullptr;
#endif
}

}  // namespace supl
