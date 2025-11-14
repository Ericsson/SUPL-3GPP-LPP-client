#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

namespace raw {
struct RxmSfrbx {
    /* 0x00 */ uint8_t gnss_id;
    /* 0x01 */ uint8_t sv_id;
    /* 0x02 */ uint8_t sig_id;
    /* 0x03 */ uint8_t freq_id;
    /* 0x04 */ uint8_t num_words;
    /* 0x05 */ uint8_t chn;
    /* 0x06 */ uint8_t version;
    /* 0x07 */ uint8_t reserved0;
};
}  // namespace raw

class Decoder;
class Encoder;
class RxmSfrbx : public Message {
public:
    CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x13;

    EXPLICIT RxmSfrbx(raw::RxmSfrbx payload, std::vector<uint32_t> words,
                      std::vector<uint8_t> data) NOEXCEPT;
    ~RxmSfrbx() override = default;

    RxmSfrbx(RxmSfrbx const& other)
        : Message(other), mPayload(other.mPayload), mWords(other.mWords) {}
    RxmSfrbx(RxmSfrbx&&)                 = delete;
    RxmSfrbx& operator=(RxmSfrbx const&) = delete;
    RxmSfrbx& operator=(RxmSfrbx&&)      = delete;

    NODISCARD const raw::RxmSfrbx& payload() const NOEXCEPT { return mPayload; }

    NODISCARD uint8_t gnss_id() const NOEXCEPT { return mPayload.gnss_id; }
    NODISCARD uint8_t sv_id() const NOEXCEPT { return mPayload.sv_id; }
    NODISCARD uint8_t sig_id() const NOEXCEPT { return mPayload.sig_id; }
    NODISCARD uint8_t freq_id() const NOEXCEPT { return mPayload.freq_id; }
    NODISCARD uint8_t num_words() const NOEXCEPT { return mPayload.num_words; }
    NODISCARD uint8_t chn() const NOEXCEPT { return mPayload.chn; }
    NODISCARD uint8_t version() const NOEXCEPT { return mPayload.version; }
    NODISCARD std::vector<uint32_t>& words() NOEXCEPT { return mWords; }

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;

private:
    raw::RxmSfrbx         mPayload;
    std::vector<uint32_t> mWords;
};

}  // namespace ubx
}  // namespace format
