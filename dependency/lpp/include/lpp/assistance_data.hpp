#pragma once
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <lpp/types.hpp>
#include <supl/cell.hpp>

#include <functional>

namespace lpp {

class Client;
struct RequestAssistanceData {
    enum class Type { OSR, SSR, AGNSS };

    Type       type;
    supl::Cell cell;

    std::function<void(Client&, Message)>                        on_non_periodic;
    std::function<void(Client&, PeriodicSessionHandle, Message)> on_periodic;
    std::function<void(Client&, PeriodicSessionHandle)>          on_started;
    std::function<void(Client&, PeriodicSessionHandle)>          on_ended;
};

}  // namespace lpp
