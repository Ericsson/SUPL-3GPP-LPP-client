#ifndef SPARTN_MESSAGE_
#define SPARTN_MESSAGE_

#include <memory>
#include <queue>

#include <generator/spartn/data.h>
#include <generator/spartn/message_header.h>
#include <generator/spartn/time.hpp>

struct SPARTN_Message {
    SPARTN_Message() : time(new SPARTN_LPP_Time) {}

    std::unique_ptr<SPARTN_Message_Header>   message_header;
    std::queue<std::unique_ptr<SPARTN_Data>> data = {};
    uint8_t                                  message_type;
    uint8_t                                  message_sub_type;
    std::unique_ptr<SPARTN_LPP_Time>         time;
};

#endif
