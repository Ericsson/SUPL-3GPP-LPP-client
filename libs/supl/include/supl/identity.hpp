#pragma once
#include <supl/types.hpp>

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

    static Identity unknown() { return Identity{.type = Identity::UNKNOWN}; }

    static Identity msisdn(int64_t id) {
        return Identity{
            .type = Identity::MSISDN,
            .data = {.msisdn = id},
        };
    }

    static Identity imsi(int64_t id) {
        return Identity{
            .type = Identity::IMSI,
            .data = {.msisdn = id},
        };
    }

    static Identity ipv4(uint8_t* data) {
        auto identity = Identity{.type = Identity::IPV4};
        memcpy(identity.data.ipv4, data, 4);
        return identity;
    }

    static Identity ipv6(uint8_t* data) {
        auto identity = Identity{.type = Identity::IPV6};
        memcpy(identity.data.ipv6, data, 16);
        return identity;
    }

    static Identity fQDN(std::string data) {
        return Identity{.type = Identity::FQDN, .data = {.fQDN = std::move(data)}};
    }
};

}  // namespace supl
