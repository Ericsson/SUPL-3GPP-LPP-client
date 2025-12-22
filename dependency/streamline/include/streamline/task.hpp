#pragma once
#include <streamline/consumer.hpp>
#include <streamline/inspector.hpp>
#include <streamline/queue.hpp>

#include <memory>
#include <unistd.h>
#include <vector>

#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE_FORWARD_REF(streamline);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(streamline)

namespace streamline {

template <typename T>
struct TypeName {
    static char const* name() { return typeid(T).name(); }
};

template <typename T>
struct Clone {
    T operator()(T& data) { return data; }
};

class QueueTaskBase {
public:
    QueueTaskBase() : mScheduler(nullptr), mEvent{scheduler::ScheduledEvent::invalid()} {}
    virtual ~QueueTaskBase()                               = default;
    virtual void schedule(scheduler::Scheduler* scheduler) = 0;
    virtual void cancel()                                  = 0;

protected:
    scheduler::Scheduler*     mScheduler;
    scheduler::ScheduledEvent mEvent;
};

template <typename T>
class QueueTask : public QueueTaskBase {
public:
    struct Item {
        uint64_t tag;
        T        data;
    };

    QueueTask(System& system) : mSystem(system), mQueue() {
        mQueueName = std::string{"streamline/"} + TypeName<T>::name();
    }
    ~QueueTask() override { cancel(); }

    void schedule(scheduler::Scheduler* scheduler) override {
        mEvent = scheduler->register_fd(
            mQueue.get_fd(), scheduler::EventInterest::Read,
            [this](scheduler::EventInterest triggered) {
                this->event(triggered);
            },
            mQueueName);

        if (mEvent.valid()) {
            VERBOSEF("queue task (%d) scheduled", mQueue.get_fd());
            mScheduler = scheduler;
        } else {
            WARNF("queue task (%d) failed to schedule", mQueue.get_fd());
            mScheduler = nullptr;
        }
    }

    void cancel() override {
        if (mScheduler) {
            mScheduler->unregister(mEvent);
            VERBOSEF("queue task (%d) cancelled", mQueue.get_fd());
            mScheduler = nullptr;
            mEvent     = scheduler::ScheduledEvent::invalid();
        }
    }

    void event(scheduler::EventInterest triggered) {
        if (!mScheduler) return;
        if (!(triggered & scheduler::EventInterest::Read)) return;

        auto count = mQueue.poll_count();
        VERBOSEF("queue task (%d): event count %lu (%zu inspectors, %zu consumers)",
                 mQueue.get_fd(), count, mInspectors.size(), mConsumers.size());
        LOGLET_INDENT_SCOPE(loglet::Level::Verbose);

        for (uint64_t i = 0; i < count; i++) {
            auto item = mQueue.pop();
            for (auto& inspector : mInspectors) {
                if (inspector->accept(mSystem, item.tag)) {
                    auto before_event = std::chrono::steady_clock::now();
                    inspector->inspect(mSystem, item.data, item.tag);
                    auto after_event = std::chrono::steady_clock::now();
                    VERBOSEF("inspector \"%s\" took %lld ms", inspector->name(),
                             std::chrono::duration_cast<std::chrono::milliseconds>(after_event -
                                                                                   before_event)
                                 .count());
                }
            }

            auto it = mConsumers.begin();
            while (it != mConsumers.end()) {
                auto consumer = it->get();
                if (!consumer->accept(mSystem, item.tag)) {
                    it++;
                    continue;
                }

                auto before_event = std::chrono::steady_clock::now();

                auto data = std::move(item.data);
                if (std::next(it) != mConsumers.end()) {
                    auto clone = streamline::Clone<T>{}(data);
                    VERBOSEF("cloning data for next consumer");
                    consumer->consume(mSystem, std::move(clone), item.tag);
                } else {
                    consumer->consume(mSystem, std::move(data), item.tag);
                }

                auto after_event = std::chrono::steady_clock::now();
                VERBOSEF("consumer \"%s\" took %lld ms", consumer->name(),
                         std::chrono::duration_cast<std::chrono::milliseconds>(after_event -
                                                                               before_event)
                             .count());

                it++;
            }
        }
    }

    void push(T&& value, uint64_t tag) { mQueue.push({tag, std::move(value)}); }

    template <typename Consumer>
    void add_consumer(std::unique_ptr<Consumer> consumer) {
        mConsumers.push_back(std::move(consumer));
    }

    template <typename Inspector>
    void add_inspector(std::unique_ptr<Inspector> inspector) {
        mInspectors.push_back(std::move(inspector));
    }

protected:
    System&                                    mSystem;
    EventQueue<Item>                           mQueue;
    std::vector<std::unique_ptr<Consumer<T>>>  mConsumers;
    std::vector<std::unique_ptr<Inspector<T>>> mInspectors;
    std::string                                mQueueName;
};

}  // namespace streamline

#undef LOGLET_CURRENT_MODULE
