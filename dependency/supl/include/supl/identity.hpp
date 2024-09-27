#pragma once
#include <core/core.hpp>

#include <string>
#include <cstring>

namespace supl {

struct Identity {
    enum Type {
        UNKNOWN = 0,
        MSISDN,
        IMSI,
        IPV4,
        IPV6,
        FQDN,
    };

    Type type;
    struct {
        int64_t     msisdn;
        int64_t     imsi;
        uint8_t     ipv4[4];
        uint8_t     ipv6[16];
        std::string fQDN;
    } data;

    static Identity unknown() { 
        Identity identity{};
        identity.type = Identity::UNKNOWN;
        return identity;
     }

    static Identity msisdn(int64_t id) {
        Identity identity{};
        identity.type = Identity::MSISDN;
        identity.data.msisdn = id;
        return identity;
    }

    static Identity imsi(int64_t id) {
        Identity identity{};
        identity.type = Identity::IMSI;
        identity.data.imsi = id;
        return identity;
    }

    static Identity ipv4(uint8_t* data) {
        Identity identity{};
        identity.type = Identity::IPV4;
        memcpy(identity.data.ipv4, data, 4);
        return identity;
    }

    static Identity ipv6(uint8_t* data) {
        Identity identity{};
        identity.type = Identity::IPV6;
        memcpy(identity.data.ipv6, data, 16);
        return identity;
    }

    static Identity fQDN(std::string data) {
        Identity identity{};
        identity.type = Identity::FQDN;
        identity.data.fQDN = std::move(data);
        return identity;
    }
};

}  // namespace supl
