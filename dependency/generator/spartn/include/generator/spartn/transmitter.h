#ifndef SPARTN_Transmitter_
#define SPARTN_Transmitter_

#include <generator/spartn/constants.h>
#include <generator/spartn/data.h>
#include <generator/spartn/message.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>

class SPARTN_Transmitter {
public:
    /** Build SPARTN binary message of internal representation SPARTN message. Message can be
     * hand crafted or generated from an LPP message using SPARTN_Generator::generate().
     *
     * NOTE: No message authentication is used, therefore TF004, TF012, TF013
     *       TF014, TF015 are all excluded.
     *
     * NOTE: CRC-16-CCITT is always used as the CRC algorithm, there is no
     *       functionality to change it implemented.
     *
     * @param[in] messages       reference to a vector of SPARTN_Message
     *                           objects that will be included in the payload.
     */
    static std::vector<uint8_t> build(std::unique_ptr<SPARTN_Message>& message);

private:
    /** Convert internal representation of SPARTN message into binary.
     *
     * SPARTN messages can be split up into the following three sections:
     *
     * +-----------------+---------+-------------+
     * |   FRAME START   | PAYLOAD |  FRAME END  |
     * | TF001 -- TF0015 |  TF016  | TF017 TF018 |
     * +-----------------+---------+-------------+
     *
     * Therefore the arguments to this function are split up into these
     * sections for easier manipulation into binary.
     *
     * @param[in] fields_to_16   all fields that should be inserted _before_
     *                           the payload (TF016).
     *
     * @param[in] fields_from_16 all fields that should be inserted _after_ the
     *                           payload (TF016).
     *
     * @param[in] payload        TF016 in binary format.
     *
     * @param[in] payload_length As the payload is stored in a bitset that has
     *                           the maximum possible width, it is also
     *                           required to know how long the actual payload
     *                           is. [units: bits]
     */
    static std::vector<uint8_t>
    output(const std::vector<std::unique_ptr<SPARTN_Field>>& fields_to_16,
           const std::vector<std::unique_ptr<SPARTN_Field>>& fields_from_16,
           const std::bitset<Constants::max_payload_size>& payload, uint64_t payload_length);

    /** Takes the internal representation of a SPARTN message and returns a
     * bitset of the same information, along with it's length.
     * SPARTN_Transmitter::add_bits_to_bitset_from_data does a lot of the
     * heavy lifting here.
     *
     * @param[in] message a pointer to the message to be converted to binary.
     *
     * @return            a pair of the passed in message represented as
     *                    binary and the length of the bitset.
     */
    static std::pair<std::bitset<Constants::max_payload_size>, uint32_t>
    generate_message_payload(std::unique_ptr<SPARTN_Message>& message);

    /** Depth-first recursion through tree-structure of a queue of SPARTN_Data
     * structs.
     *
     * This function will pop the first element of the queue and check it's
     * type:
     *
     * - SPARTN_Block: call this function again, passing in the block's queue
     *                 of data.
     *
     * - SPARTN_Field: add the field's bits to the payload and return.
     *
     * WARN: this function modifies the `data` object, by popping off values
     *       from the queue and releasing the data inside of the fields.
     *       Therefore, any `data` passed into this function should not be
     *       used afterwards.
     *
     * WARN: this function does not truncate values that are beyond the
     *       specified bit count, which can lead to corruption if either field
     *       is set incorrectly.
     *
     * @param[out] payload   the payload to be created.
     *
     * @param[in]  data      a reference to a queue of SPARTN_Data objects,
     *                       typically from either a SPARTN_Message or
     *                       SPARTN_Block.
     *
     * @param[out] bits_used the length of the resulting payload.
     *                       [units: bits]
     */
    template <size_t S>
    static void add_bits_to_bitset_from_data(std::bitset<S>&                           payload,
                                             std::queue<std::unique_ptr<SPARTN_Data>>& data,
                                             uint64_t&                                 bits_used);

    /** Generates the TF006 field - the frame CRC.
     *
     * This function does not calculate the CRC-4 directly, it sets up the data
     * structure, calls SPARTN_Transmitter::generate_crc4_for_tf006() and then
     * creates a SPARTN_Field to be returned.
     *
     * @param[in] TF002 message type field.
     * @param[in] TF003 payload length.
     * @param[in] TF004 encryption and authentication flag.
     * @param[in] TF005 message CRC type.
     *
     * @return          TF006 with the CRC-4 calculated.
     */
    static std::unique_ptr<SPARTN_Field> generate_tf006(const std::unique_ptr<SPARTN_Field>& TF002,
                                                        const std::unique_ptr<SPARTN_Field>& TF003,
                                                        const std::unique_ptr<SPARTN_Field>& TF004,
                                                        const std::unique_ptr<SPARTN_Field>& TF005);

    /** Generates CRC-16-CCITT from SPARTN_Field structs and the payload.
     *
     * @param[in] fields_02_to_16 SPARTN fields TF002 up-to and including
     *                            TF015.
     * @param[in] payload         TF016 as a bitset.
     * @param[in] payload_length  As the payload is stored in a bitset that has
     *                            the maximum possible width, it is also
     *                            required to know how long the actual payload
     *                            is. [units: bits]
     *
     * @return                    calculated CRC-16
     */
    static uint16_t
    generate_crc16(const std::vector<std::unique_ptr<SPARTN_Field>>& fields_02_to_16,
                   const std::bitset<Constants::max_payload_size>&   payload,
                   uint64_t                                          payload_length);

    /** Opens a file descriptor to the desired path.
     *
     * If the path is /dev/ttyS0 then a UART connection will be set up.
     *  Baudrate: 460800
     * If the path is /dev/i2c-1 then a I2C connection will be setup.
     *  Slave address: 0x42
     *
     * The serial device is expected to be accessible through the /dev/ttyS0
     * device (default on the Raspberry Pi 4). Due to the size of some of the
     * messages, baudrate must be high (460800 will work, but 38400 won't for
     * example).
     */
    static int open_serial(const char* path);

    /** Given a payload and a list of fields, add all the bits from the fields
     * to the payload.
     *
     * @param[out] payload   a reference to the bitset to be modified.
     *
     * @param[in]  fields    a reference to the list of fields to have their
     *                       bits appended to the payload.
     *
     * @param[out] bits_used number of bits used in the payload.
     */
    template <size_t S>
    static void add_bits_to_bitset(std::bitset<S>&                                   payload,
                                   const std::vector<std::unique_ptr<SPARTN_Field>>& fields,
                                   uint64_t&                                         bits_used) {
        for (const auto& field : fields) {
            payload <<= field->bit_count;
            payload |= std::bitset<S>(field->bits.to_string());

            bits_used += field->bit_count;
        }
    }

    /** Calculate the CRC-4 value for the SPARTN field TF006.
     *
     * @param[in] bytes a reference to the three bytes used in the calculation.
     *
     * @return          the CRC-4 value.
     */
    static uint8_t generate_crc4_for_tf006(const std::array<uint8_t, 3>& bytes) {
        uint8_t crc = 0;
        for (const auto byte : bytes) {
            const uint8_t iCrc = byte ^ crc;
            crc                = Constants::crc4_lookuptable[iCrc];
            crc &= 0xF;
        }
        return crc;
    }

    /** Given a bitset and a position (in bytes), return the byte at that
     * position.
     *
     * @param[in] bs            reference to the bitset to a byte extracted
     *                          from.
     *
     * @param[in] bits_to_shift position of where the byte should be extracted
     *                          from. [units: bits]
     *
     * @return                  extracted byte.
     */
    template <size_t S>
    static uint8_t get_byte(const std::bitset<S>& bs, uint64_t bits_to_shift) {
        constexpr auto mask = std::bitset<S>(0xFF);
        return std::bitset<S>((bs >> bits_to_shift) & mask).to_ulong();
    }
};
#endif
