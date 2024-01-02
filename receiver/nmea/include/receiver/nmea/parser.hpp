#pragma once
#include <memory>
#include <receiver/nmea/types.hpp>

namespace receiver {
namespace nmea {

class Message;
class Parser {
public:
    NMEA_EXPLICIT Parser() NMEA_NOEXCEPT;
    ~Parser() NMEA_NOEXCEPT;

    bool append(uint8_t* data, uint16_t length) NMEA_NOEXCEPT;
    void clear() NMEA_NOEXCEPT;

    NMEA_NODISCARD std::unique_ptr<Message> try_parse() NMEA_NOEXCEPT;
    NMEA_NODISCARD uint32_t                 buffer_length() const NMEA_NOEXCEPT;
    NMEA_NODISCARD uint32_t                 available_space() const NMEA_NOEXCEPT;

    NMEA_NODISCARD static uint16_t checksum_message(const uint8_t* message_data,
                                                    uint16_t       message_length);
    NMEA_NODISCARD static uint16_t checksum(const uint8_t* payload, uint16_t length);

protected:
    NMEA_NODISCARD uint8_t peek(uint32_t index) const NMEA_NOEXCEPT;
    void                   skip(uint32_t length) NMEA_NOEXCEPT;
    void                   copy_to_buffer(uint8_t* data, uint32_t length) NMEA_NOEXCEPT;

    NMEA_NODISCARD std::string parse_prefix(const uint8_t* data,
                                            uint32_t       length) const NMEA_NOEXCEPT;

private:
    uint8_t* mBuffer;
    uint32_t mBufferCapacity;
    uint32_t mBufferRead;
    uint32_t mBufferWrite;
};

}  // namespace nmea
}  // namespace receiver
