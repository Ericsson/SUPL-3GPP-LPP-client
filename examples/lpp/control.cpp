#include "control.hpp"

#include <cstring>
#include <sstream>
#include <vector>

static std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void ControlParser::parse(std::unique_ptr<interface::Interface>& interface) {
    while (interface->can_read()) {
        char   buffer[128];
        size_t length = interface->read(buffer, sizeof(buffer));
        if (length <= 0) {
            break;
        }

        auto left = sizeof(mBuffer) - mLength;
        if (length > left) {
            length = left;
        }

        std::memcpy(mBuffer + mLength, buffer, length);
        mLength += length;
    }

    auto lookahead_index = 0;
    while (mLength - lookahead_index > 3) {
        if (mBuffer[lookahead_index] != '/') {
            lookahead_index++;
            continue;
        }

        auto message_length = 0;
        for (auto i = lookahead_index + 1; i < mLength; i++) {
            if (mBuffer[i] == '\r' && i + 1 < mLength && mBuffer[i + 1] == '\n') {
                message_length = i - lookahead_index + 2;
                break;
            }
        }

        if (message_length == 0) {
            break;
        }

        std::string message(mBuffer + lookahead_index + 1, message_length - 3);
        auto        tokens = split(message, ',');
        if (tokens.size() > 0) {
            if (tokens[0] == "CID") {
                if (tokens.size() == 6) {
                    CellID cid;
                    cid.mcc   = std::stoi(tokens[2]);
                    cid.mnc   = std::stoi(tokens[3]);
                    cid.tac   = std::stoi(tokens[4]);
                    cid.cell  = std::stoi(tokens[5]);
                    cid.is_nr = tokens[1] == "N";
                    if (on_cid) {
                        on_cid(cid);
                    }
                } else {
                    printf("invalid CID message: %s\n", message.c_str());
                }
            } else {
                printf("unknown command: %s\n", message.c_str());
            }
        }

        lookahead_index += message_length;
    }

    if (lookahead_index > 0) {
        mLength -= lookahead_index;
        std::memmove(mBuffer, mBuffer + lookahead_index, mLength);
    }
}
