#include "transmitter.h"

std::vector<uint8_t> SPARTN_Transmitter::build(std::unique_ptr<SPARTN_Message>& message) {
    std::cout << "Message ";
    std::cout << " of type (" << (int)message->message_type << ", ";
    std::cout << (int)message->message_sub_type << ")";

    const std::pair<std::bitset<Constants::max_payload_size>, uint32_t> payload_size =
        SPARTN_Transmitter::generate_message_payload(message);

    std::unique_ptr<SPARTN_Field> TF001(new SPARTN_Field{1, 8, 115});

    std::unique_ptr<SPARTN_Field> TF002(new SPARTN_Field{2, 7, message->message_type});

    std::unique_ptr<SPARTN_Field> TF003(new SPARTN_Field{3, 10, payload_size.second});

    if (payload_size.second > 1024) {
        std::cout << std::endl;
        std::cout << "WARN: sending a payload of size than max: ";
        std::cout << payload_size.second << " > "
                  << "1024" << std::endl;
    }

    std::unique_ptr<SPARTN_Field> TF004(new SPARTN_Field{4, 1, 0});

    // CRC-16-CCITT
    std::unique_ptr<SPARTN_Field> TF005(new SPARTN_Field{5, 2, 1});

    std::unique_ptr<SPARTN_Field> TF006 =
        SPARTN_Transmitter::generate_tf006(TF002, TF003, TF004, TF005);

    std::unique_ptr<SPARTN_Field> TF007(new SPARTN_Field{7, 4, message->message_sub_type});

    std::unique_ptr<SPARTN_Field> TF008(new SPARTN_Field{8, 1, 1});

    std::unique_ptr<SPARTN_Field> TF009(new SPARTN_Field{9, 32, message->time->get_spartn_time()});

    std::unique_ptr<SPARTN_Field> TF010(new SPARTN_Field{10, 7, 0});

    std::unique_ptr<SPARTN_Field> TF011(new SPARTN_Field{11, 4, 0});

    std::vector<std::unique_ptr<SPARTN_Field>> to_16 = {};
    to_16.push_back(std::move(TF002));
    to_16.push_back(std::move(TF003));
    to_16.push_back(std::move(TF004));
    to_16.push_back(std::move(TF005));
    to_16.push_back(std::move(TF006));
    to_16.push_back(std::move(TF007));
    to_16.push_back(std::move(TF008));
    to_16.push_back(std::move(TF009));
    to_16.push_back(std::move(TF010));
    to_16.push_back(std::move(TF011));

    std::unique_ptr<SPARTN_Field> TF018(new SPARTN_Field{
        18, 16,
        SPARTN_Transmitter::generate_crc16(to_16, payload_size.first, payload_size.second)});

    // crc16 calc doesnt want TF001, but it is needed for bin file
    // so move it in after the crc is calculated
    to_16.insert(to_16.begin(), std::move(TF001));

    std::vector<std::unique_ptr<SPARTN_Field>> from_16 = {};
    from_16.push_back(std::move(TF018));

    auto output =
        SPARTN_Transmitter::output(to_16, from_16, payload_size.first, payload_size.second);

    std::cout << std::endl;

    return output;
}

/* Returns: **byte-algined** payload and length of payload (in bytes) */
std::pair<std::bitset<Constants::max_payload_size>, uint32_t>
SPARTN_Transmitter::generate_message_payload(std::unique_ptr<SPARTN_Message>& message) {
    std::bitset<Constants::max_payload_size> payload;
    uint64_t                                 current_length = 0;

    SPARTN_Transmitter::add_bits_to_bitset_from_data(payload, message->message_header->fields,
                                                     current_length);
    SPARTN_Transmitter::add_bits_to_bitset_from_data(payload, message->data, current_length);

    const uint64_t shift_amount = (8 - (current_length % 8)) % 8;
    current_length += shift_amount;
    payload <<= shift_amount;  // byte align

    return std::make_pair(payload, current_length / 8);
}

std::vector<uint8_t>
SPARTN_Transmitter::output(const std::vector<std::unique_ptr<SPARTN_Field>>& fields_to_16,
                           const std::vector<std::unique_ptr<SPARTN_Field>>& fields_from_16,
                           const std::bitset<Constants::max_payload_size>&   payload,
                           const uint64_t                                    payload_length) {
    std::bitset<Constants::max_message_size> message(0);
    uint64_t                                 message_size = 0;

    SPARTN_Transmitter::add_bits_to_bitset(message, fields_to_16, message_size);

    // moving the payload into position
    message <<= (payload_length * 8);
    message |= std::bitset<Constants::max_message_size>(payload.to_string());
    message_size += (payload_length * 8);

    SPARTN_Transmitter::add_bits_to_bitset(message, fields_from_16, message_size);

    const uint64_t shift_amount              = (8 - (message_size % 8)) % 8;
    const int64_t  byte_aligned_message_size = (int64_t)(message_size + shift_amount);
    const std::bitset<Constants::max_message_size> byte_aligned_message = message << shift_amount;

    /*
     * Seems as if i2c doesn't like one byte being stored at a time, sending
     * two at once it does though.
     */
    static constexpr uint8_t buff_size                = 2;
    char                     bytes_to_send[buff_size] = {};
    uint8_t                  byte_to_write            = 0;
    std::vector<uint8_t>     output;

    for (ssize_t i = (byte_aligned_message_size / 8) - 1; i >= 0; i--) {
        const uint64_t byte_i    = i * 8;
        const uint8_t  this_byte = SPARTN_Transmitter::get_byte(byte_aligned_message, byte_i);

        bytes_to_send[byte_to_write] = (char)this_byte;
        if ((++byte_to_write % 2) == 0) {
            byte_to_write = 0;
            for (ssize_t j = 0; j < buff_size; j++) {
                output.push_back(bytes_to_send[j]);
            }
        }
    }

    if (byte_to_write == 1) {
        bytes_to_send[byte_to_write] = 0x00;
        ++byte_to_write;
        for (ssize_t j = 0; j < buff_size; j++) {
            output.push_back(bytes_to_send[j]);
        }
    }

    std::cout << " sent: " << (byte_aligned_message_size / 8) - 1 << "(" << output.size()
              << ") bytes";
    return output;
}

/* Do not use any values from data after this function is called */
template <size_t S>
void SPARTN_Transmitter::add_bits_to_bitset_from_data(
    std::bitset<S>& payload, std::queue<std::unique_ptr<SPARTN_Data>>& data, uint64_t& bits_used) {
    // std::cout << std::endl;
    while (!data.empty()) {
        switch (data.front()->get_data_type()) {
        case SPARTN_Data::DataType::Block: {
            SPARTN_Data*  d_ptr = data.front().get();
            SPARTN_Block* b_ptr = static_cast<SPARTN_Block*>(d_ptr);

            SPARTN_Transmitter::add_bits_to_bitset_from_data(payload, b_ptr->data, bits_used);

            break;
        }
        case SPARTN_Data::DataType::Field: {
            SPARTN_Data*  d_ptr = data.front().get();
            SPARTN_Field* f_ptr = static_cast<SPARTN_Field*>(d_ptr);

            // bear in mind that if the bits are set beyond specified bit_count
            // (it is not guarrenteed that this will never be the case) then
            // previous bits will be overwritten.
            payload <<= f_ptr->bit_count;
            payload |= std::bitset<S>(f_ptr->bits.to_string());

            // std::cout << "SF" << (int)f_ptr->id << "/";

            if (f_ptr->bit_count <= 64) {
                // std::cout << f_ptr->bits.to_ullong();
                if (f_ptr->bits.to_ullong() > powl(2, f_ptr->bit_count)) {
                    std::cout << "[WARN] SF" << (int)f_ptr->id << " has value greater than legal"
                              << std::endl;
                }
            }
            // else
            // {
            //     std::cout << f_ptr->bits.to_string();
            // }
            // std::cout << " ";

            bits_used += f_ptr->bit_count;

            break;
        }
        }
        data.pop();
    }
}

std::unique_ptr<SPARTN_Field> SPARTN_Transmitter::generate_tf006(
    const std::unique_ptr<SPARTN_Field>& TF002, const std::unique_ptr<SPARTN_Field>& TF003,
    const std::unique_ptr<SPARTN_Field>& TF004, const std::unique_ptr<SPARTN_Field>& TF005) {
    // concatenate all the bits into one variable - 24 bits long
    std::bitset<Constants::max_spartn_bit_count> tf2to5 = TF002->bits;
    tf2to5 <<= TF003->bit_count;
    tf2to5 |= TF003->bits;
    tf2to5 <<= TF004->bit_count;
    tf2to5 |= TF004->bits;
    tf2to5 <<= TF005->bit_count;
    tf2to5 |= TF005->bits;

    /* SPARTN spec calls for an extra 4 zeros to be added for the bitstring
     * to be byte aligned */
    tf2to5 <<= 4;

    static constexpr std::bitset<Constants::max_spartn_bit_count> mask = 0xFF;
    // get each first 3 bytes out of the bit string
    const uint8_t crc4 = SPARTN_Transmitter::generate_crc4_for_tf006(
        {(uint8_t)((tf2to5 >> (size_t)(2 * 8)) & mask).to_ulong(),
         (uint8_t)((tf2to5 >> (size_t)(1 * 8)) & mask).to_ulong(),
         (uint8_t)((tf2to5 >> (size_t)(0 * 8)) & mask).to_ulong()});

    std::unique_ptr<SPARTN_Field> TF006(new SPARTN_Field{6, 4, crc4});
    return TF006;
}

uint16_t SPARTN_Transmitter::generate_crc16(
    const std::vector<std::unique_ptr<SPARTN_Field>>& fields_02_to_16,
    const std::bitset<Constants::max_payload_size>& payload, const uint64_t payload_length) {
    std::bitset<Constants::max_message_size> message(0);
    uint64_t                                 message_size = 0;

    SPARTN_Transmitter::add_bits_to_bitset(message, fields_02_to_16, message_size);

    message <<= (payload_length * 8);
    message |= std::bitset<Constants::max_message_size>(payload.to_string());
    message_size += (payload_length * 8);

    uint16_t crc = 0;

    const uint8_t shift_amount              = (8 - (message_size % 8)) % 8;
    const int64_t byte_aligned_message_size = (int64_t)(message_size + shift_amount);
    const std::bitset<Constants::max_message_size> byte_aligned_message = message << shift_amount;

    for (ssize_t i = (byte_aligned_message_size / 8) - 1; i >= 0; i--) {
        const uint64_t byte_i    = i * 8;
        const uint8_t  this_byte = SPARTN_Transmitter::get_byte(byte_aligned_message, byte_i);
        const uint16_t iCrc      = this_byte ^ (crc >> 8);
        crc                      = Constants::lib_crc_kCrc16qtable[iCrc] ^ (crc << 8);
    }

    std::cout << " got CRC-16 of: " << (int)crc;
    return crc;
}

int SPARTN_Transmitter::open_serial(const char* path) {
    int fd;
    if (strcmp(path, "/dev/ttyS0") == 0) {
        struct termios        options;
        static constexpr auto baudrate(460800);

        fd = open("/dev/ttyS0", O_WRONLY | O_NOCTTY);
        if (fd == -1) return -1;

        fcntl(fd, F_SETFL, 0);
        tcgetattr(fd, &options);
        cfsetospeed(&options, baudrate);
        cfmakeraw(&options);
        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~CRTSCTS;
        if (tcsetattr(fd, TCSANOW, &options) != 0) return -1;
    } else if (strcmp(path, "/dev/i2c-1") == 0) {
        static constexpr uint8_t i2c_slave_adrr = 0x42;
        fd                                      = open("/dev/i2c-1", O_RDWR);
        if (fd == -1) {
            return -1;
        }
        if (ioctl(fd, I2C_SLAVE, i2c_slave_adrr) < 0) {
            std::cerr << "ERR: Could not open i2c interface" << std::endl;
            return -1;
        }
    } else {
        fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    }

    return fd;
}
