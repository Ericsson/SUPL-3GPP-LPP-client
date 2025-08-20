#pragma once
#include <core/core.hpp>

namespace format {
namespace ubx {

class Decoder;

using CfgLayer                            = uint8_t;
CONSTEXPR static CfgLayer CFG_LAYER_RAM   = 0x01;
CONSTEXPR static CfgLayer CFG_LAYER_BBR   = 0x02;
CONSTEXPR static CfgLayer CFG_LAYER_FLASH = 0x04;

using CfgKey = uint32_t;

CONSTEXPR static CfgKey CFG_KEY_RATE_MEAS        = 0x30210001;
CONSTEXPR static CfgKey CFG_KEY_NAVHPG_DGNSSMODE = 0x20140011;

CONSTEXPR static CfgKey CFG_KEY_UART1_ENABLED       = 0x10520005;
CONSTEXPR static CfgKey CFG_KEY_UART1_BAUDRATE      = 0x40520001;
CONSTEXPR static CfgKey CFG_KEY_UART1_STOPBITS      = 0x20520002;
CONSTEXPR static CfgKey CFG_KEY_UART1_DATABITS      = 0x20520003;
CONSTEXPR static CfgKey CFG_KEY_UART1_PARITY        = 0x20520004;
CONSTEXPR static CfgKey CFG_KEY_UART1INPROT_UBX     = 0x10730001;
CONSTEXPR static CfgKey CFG_KEY_UART1INPROT_NMEA    = 0x10730002;
CONSTEXPR static CfgKey CFG_KEY_UART1INPROT_RTCM3X  = 0x10730004;
CONSTEXPR static CfgKey CFG_KEY_UART1INPROT_SPARTN  = 0x10730005;
CONSTEXPR static CfgKey CFG_KEY_UART1OUTPROT_UBX    = 0x10740001;
CONSTEXPR static CfgKey CFG_KEY_UART1OUTPROT_NMEA   = 0x10740002;
CONSTEXPR static CfgKey CFG_KEY_UART1OUTPROT_RTCM3X = 0x10740004;
CONSTEXPR static CfgKey CFG_KEY_UART2_ENABLED       = 0x10530005;
CONSTEXPR static CfgKey CFG_KEY_UART2_BAUDRATE      = 0x40530001;
CONSTEXPR static CfgKey CFG_KEY_UART2_STOPBITS      = 0x20530002;
CONSTEXPR static CfgKey CFG_KEY_UART2_DATABITS      = 0x20530003;
CONSTEXPR static CfgKey CFG_KEY_UART2_PARITY        = 0x20530004;
CONSTEXPR static CfgKey CFG_KEY_UART2INPROT_UBX     = 0x10750001;
CONSTEXPR static CfgKey CFG_KEY_UART2INPROT_NMEA    = 0x10750002;
CONSTEXPR static CfgKey CFG_KEY_UART2INPROT_RTCM3X  = 0x10750004;
CONSTEXPR static CfgKey CFG_KEY_UART2INPROT_SPARTN  = 0x10750005;
CONSTEXPR static CfgKey CFG_KEY_UART2OUTPROT_UBX    = 0x10760001;
CONSTEXPR static CfgKey CFG_KEY_UART2OUTPROT_NMEA   = 0x10760002;
CONSTEXPR static CfgKey CFG_KEY_UART2OUTPROT_RTCM3X = 0x10760004;
CONSTEXPR static CfgKey CFG_KEY_I2C_ENABLED         = 0x10510003;
CONSTEXPR static CfgKey CFG_KEY_I2C_ADDRESS         = 0x20510001;
CONSTEXPR static CfgKey CFG_KEY_I2CINPROT_UBX       = 0x10710001;
CONSTEXPR static CfgKey CFG_KEY_I2CINPROT_NMEA      = 0x10710002;
CONSTEXPR static CfgKey CFG_KEY_I2CINPROT_RTCM3X    = 0x10710004;
CONSTEXPR static CfgKey CFG_KEY_I2CINPROT_SPARTN    = 0x10710005;
CONSTEXPR static CfgKey CFG_KEY_I2COUTPROT_UBX      = 0x10720001;
CONSTEXPR static CfgKey CFG_KEY_I2COUTPROT_NMEA     = 0x10720002;
CONSTEXPR static CfgKey CFG_KEY_I2COUTPROT_RTCM3X   = 0x10720004;
CONSTEXPR static CfgKey CFG_KEY_USB_ENABLED         = 0x10650001;
CONSTEXPR static CfgKey CFG_KEY_USBINPROT_UBX       = 0x10770001;
CONSTEXPR static CfgKey CFG_KEY_USBINPROT_NMEA      = 0x10770002;
CONSTEXPR static CfgKey CFG_KEY_USBINPROT_RTCM3X    = 0x10770004;
CONSTEXPR static CfgKey CFG_KEY_USBINPROT_SPARTN    = 0x10770005;
CONSTEXPR static CfgKey CFG_KEY_USBOUTPROT_UBX      = 0x10780001;
CONSTEXPR static CfgKey CFG_KEY_USBOUTPROT_NMEA     = 0x10780002;
CONSTEXPR static CfgKey CFG_KEY_USBOUTPROT_RTCM3X   = 0x10780004;

CONSTEXPR static CfgKey CFG_KEY_MSGOUT_NAV_PVT_UART1 = 0x20910007;
CONSTEXPR static CfgKey CFG_KEY_MSGOUT_NAV_PVT_UART2 = 0x20910008;
CONSTEXPR static CfgKey CFG_KEY_MSGOUT_NAV_PVT_I2C   = 0x20910006;
CONSTEXPR static CfgKey CFG_KEY_MSGOUT_NAV_PVT_USB   = 0x20910009;

CONSTEXPR static CfgKey CFG_KEY_INFMSG_UART1 = 0x20920002;
CONSTEXPR static CfgKey CFG_KEY_INFMSG_UART2 = 0x20920003;
CONSTEXPR static CfgKey CFG_KEY_INFMSG_I2C   = 0x20920001;
CONSTEXPR static CfgKey CFG_KEY_INFMSG_USB   = 0x20920004;

CONSTEXPR static uint8_t INF_ERROR   = 0x01;
CONSTEXPR static uint8_t INF_WARNING = 0x02;
CONSTEXPR static uint8_t INF_NOTICE  = 0x04;
CONSTEXPR static uint8_t INF_TEST    = 0x08;
CONSTEXPR static uint8_t INF_DEBUG   = 0x10;

class Encoder;
struct CfgValue {
    enum Type {
        UNKNOWN,
        L,
        U1,
        U2,
        U4,
        U8,
    };

    EXPLICIT CfgValue() NOEXCEPT : mType(UNKNOWN) {}

    NODISCARD static CfgValue from_l(bool value) NOEXCEPT;
    NODISCARD static CfgValue from_u1(uint8_t value) NOEXCEPT;
    NODISCARD static CfgValue from_u2(uint16_t value) NOEXCEPT;
    NODISCARD static CfgValue from_u4(uint32_t value) NOEXCEPT;
    NODISCARD static CfgValue from_u8(uint64_t value) NOEXCEPT;

    NODISCARD static CfgValue parse_from_key(CfgKey key, Decoder& decoder) NOEXCEPT;
    NODISCARD static CfgValue parse_from_type(Type type, Decoder& decoder) NOEXCEPT;
    NODISCARD static Type     type_from_key(CfgKey key) NOEXCEPT;
    NODISCARD static uint32_t size_from_type(Type type) NOEXCEPT;

    NODISCARD Type type() const NOEXCEPT { return mType; }

    NODISCARD bool     l() const NOEXCEPT;
    NODISCARD uint8_t  u1() const NOEXCEPT;
    NODISCARD uint16_t u2() const NOEXCEPT;
    NODISCARD uint32_t u4() const NOEXCEPT;
    NODISCARD uint64_t u8() const NOEXCEPT;

    NODISCARD uint32_t size() const NOEXCEPT;
    void               serialize(Encoder& encoder) const NOEXCEPT;

private:
    EXPLICIT CfgValue(Type type, bool value) NOEXCEPT;
    EXPLICIT CfgValue(Type type, uint8_t value) NOEXCEPT;
    EXPLICIT CfgValue(Type type, uint16_t value) NOEXCEPT;
    EXPLICIT CfgValue(Type type, uint32_t value) NOEXCEPT;
    EXPLICIT CfgValue(Type type, uint64_t value) NOEXCEPT;

    Type mType;
    union {
        bool     mL;
        uint8_t  mU1;
        uint16_t mU2;
        uint32_t mU4;
        uint64_t mU8;
    } mValue;
};

}  // namespace ubx
}  // namespace format
