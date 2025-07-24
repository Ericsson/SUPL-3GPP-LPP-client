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
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(streamline)

namespace streamline {

template <typename T>
struct Clone {
    T operator()(T& data) { return data; }
};

class QueueTaskBase {
public:
    QueueTaskBase() : mScheduler(nullptr) {}
    virtual ~QueueTaskBase()                               = default;
    virtual void schedule(scheduler::Scheduler* scheduler) = 0;
    virtual void cancel()                                  = 0;

protected:
    scheduler::Scheduler* mScheduler;
};

template <typename T>
class QueueTask : public QueueTaskBase {
public:
    struct Item {
        uint64_t tag;
        T        data;
    };

    QueueTask(System& system) : mSystem(system), mQueue() {
        mEvent.name  = "streamline-queue";
        mEvent.event = [this](struct epoll_event* event) {
            this->event(event);
        };
    }
    ~QueueTask() override { cancel(); }

    void schedule(scheduler::Scheduler* scheduler) override {
        if (scheduler->add_epoll_fd(mQueue.get_fd(), EPOLLIN, &mEvent)) {
            VERBOSEF("queue task (%d) scheduled", mQueue.get_fd());
            mScheduler = scheduler;
        } else {
            WARNF("queue task (%d) failed to schedule", mQueue.get_fd());
            mScheduler = nullptr;
        }
    }

    void cancel() override {
        if (mScheduler) {
            if (mScheduler->remove_epoll_fd(mQueue.get_fd())) {
                VERBOSEF("queue task (%d) cancelled", mQueue.get_fd());
            } else {
                WARNF("queue task (%d) failed to cancel", mQueue.get_fd());
            }
            mScheduler = nullptr;
        }
    }

    void event(struct epoll_event* event) {
        if (!mScheduler) return;
        if ((event->events & EPOLLIN) == 0) return;

        auto count = mQueue.poll_count();
        VERBOSEF("queue task (%d): event count %lu", mQueue.get_fd(), count);
        LOGLET_INDENT_SCOPE(loglet::Level::Verbose);

        for (uint64_t i = 0; i < count; i++) {
            auto item = mQueue.pop();
            for (auto& inspector : mInspectors) {
                if (inspector->accept(mSystem, item.tag)) {
                    inspector->inspect(mSystem, item.data, item.tag);
                }
            }

            auto it = mConsumers.begin();
            while (it != mConsumers.end()) {
                auto consumer = it->get();
                if (!consumer->accept(mSystem, item.tag)) {
                    it++;
                    continue;
                }

                // TODO(ewasjon): This is a bit weird, if the item is consumed why clone it? What is
                // the reason for multiple consumers, instead of just using inspectors?
                auto data = std::move(item.data);
                if (std::next(it) != mConsumers.end()) {
                    auto clone = streamline::Clone<T>{}(data);
                    VERBOSEF("cloning data for next consumer");
                    consumer->consume(mSystem, std::move(clone), item.tag);
                } else {
                    consumer->consume(mSystem, std::move(data), item.tag);
                }
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
    scheduler::EpollEvent                      mEvent;
    EventQueue<Item>                           mQueue;
    std::vector<std::unique_ptr<Consumer<T>>>  mConsumers;
    std::vector<std::unique_ptr<Inspector<T>>> mInspectors;
};

}  // namespace streamline

#undef LOGLET_CURRENT_MODULE
