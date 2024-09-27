#ifndef SPARTN_MESSAGE_HEADER_
#define SPARTN_MESSAGE_HEADER_

#include <memory>
#include <vector>

#include <generator/spartn/data.h>

struct SPARTN_Message_Header {
    std::queue<std::unique_ptr<SPARTN_Data>> fields = {};
};

#endif
