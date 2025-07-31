#pragma once
#include <streamline/task.hpp>

#include <memory>
#include <mutex>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE_FORWARD_REF(streamline);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(streamline)

namespace streamline {
class System {
public:
    System() : mScheduler(nullptr) {}
    System(scheduler::Scheduler& scheduler) : mScheduler(&scheduler) {}

    ~System() { cancel(); }

    System(System const&)            = delete;
    System(System&&)                 = delete;
    System& operator=(System const&) = delete;

    System& operator=(System&& other) {
        if (this != &other) {
            cancel();
            mScheduler       = other.mScheduler;
            mQueues          = std::move(other.mQueues);
            other.mScheduler = nullptr;
        }
        return *this;
    }

    template <typename Consumer, typename... Args>
    void add_consumer(Args&&... args) {
        FUNCTION_SCOPE();
        using DataType = typename Consumer::DataType;
        VERBOSEF("add consumer %s (%s)", typeid(Consumer).name(), typeid(DataType).name());

        if (!mScheduler) {
            WARNF("invalid system state");
            return;
        }

        auto queue = get_or_create_queue<DataType>();
        if (queue) {
            queue->add_consumer(
                std::unique_ptr<Consumer>(new Consumer(std::forward<Args>(args)...)));
        }
    }

    template <typename Inspector, typename... Args>
    Inspector* add_inspector(Args&&... args) {
        FUNCTION_SCOPE();
        using DataType = typename Inspector::DataType;
        VERBOSEF("add inspector %s (%s)", typeid(Inspector).name(), typeid(DataType).name());

        if (!mScheduler) {
            WARNF("invalid system state");
            return nullptr;
        }

        auto queue = get_or_create_queue<DataType>();
        if (queue) {
            auto inspector = new Inspector(std::forward<Args>(args)...);
            queue->add_inspector(std::unique_ptr<Inspector>(inspector));
            return inspector;
        } else {
            return nullptr;
        }
    }

    template <typename DataType>
    void push(DataType&& data, uint64_t tag = 0) {
        FUNCTION_SCOPE();
        VERBOSEF("push %s", typeid(DataType).name());
        if (!mScheduler) {
            WARNF("invalid system state");
            return;
        }

        auto queue = get_queue<DataType>();
        if (queue) {
            queue->push(std::move(data), tag);
        }
    }

    void cancel() {
        FUNCTION_SCOPE();
        for (auto& it : mQueues) {
            auto queue = static_cast<QueueTaskBase*>(it.second.get());
            queue->cancel();
        }
        mScheduler = nullptr;
    }

protected:
    template <typename DataType>
    QueueTask<DataType>* get_or_create_queue() {
        FUNCTION_SCOPE();
        auto it = mQueues.find(std::type_index(typeid(DataType)));
        if (it == mQueues.end()) {
            VERBOSEF("created queue for %s", typeid(DataType).name());
            auto queue = std::shared_ptr<QueueTask<DataType>>(new QueueTask<DataType>(*this));
            queue->schedule(mScheduler);
            mQueues[std::type_index(typeid(DataType))] = std::move(queue);
        }

        auto queue =
            static_cast<QueueTask<DataType>*>(mQueues[std::type_index(typeid(DataType))].get());
        return queue;
    }

    template <typename DataType>
    QueueTask<DataType>* get_queue() {
        FUNCTION_SCOPE();
        auto it = mQueues.find(std::type_index(typeid(DataType)));
        if (it != mQueues.end()) {
            return static_cast<QueueTask<DataType>*>(it->second.get());
        }
        return nullptr;
    }

private:
    scheduler::Scheduler*                                      mScheduler;
    std::unordered_map<std::type_index, std::shared_ptr<void>> mQueues;
};
}  // namespace streamline

#undef LOGLET_CURRENT_MODULE
