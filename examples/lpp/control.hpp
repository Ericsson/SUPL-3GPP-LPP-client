#pragma once
#include <interface/interface.hpp>
#include <lpp/cell_id.h>

#include <memory>
#include <functional>

class ControlParser {
public:
    std::function<void(CellID)> on_cid;

    void parse(std::unique_ptr<interface::Interface>& interface);

private:
    char   mBuffer[4096];
    size_t mLength;
};
