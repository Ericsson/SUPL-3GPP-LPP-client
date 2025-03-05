#pragma once
#include <streamline/consumer.hpp>
#include <streamline/inspector.hpp>
#include <streamline/queue.hpp>

#include <memory>
#include <unistd.h>
#include <vector>

#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

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
    QueueTask(System& system) : mSystem(system), mQueue() {
        mEvent.name  = "streamline-queue";
        mEvent.event = [this](struct epoll_event* event) {
            this->event(event);
        };
    }
    ~QueueTask() override { cancel(); }

    void schedule(scheduler::Scheduler* scheduler) override {
        if (scheduler->add_epoll_fd(mQueue.get_fd(), EPOLLIN, &mEvent)) {
            XVERBOSEF("smtl", "queue task (%d) scheduled", mQueue.get_fd());
            mScheduler = scheduler;
        } else {
            XWARNF("smtl", "queue task (%d) failed to schedule", mQueue.get_fd());
            mScheduler = nullptr;
        }
    }

    void cancel() override {
        if (mScheduler) {
            if (mScheduler->remove_epoll_fd(mQueue.get_fd())) {
                XVERBOSEF("smtl", "queue task (%d) cancelled", mQueue.get_fd());
            } else {
                XWARNF("smtl", "queue task (%d) failed to cancel", mQueue.get_fd());
            }
            mScheduler = nullptr;
        }
    }

    void event(struct epoll_event* event) {
        if (!mScheduler) return;
        if ((event->events & EPOLLIN) == 0) return;

        auto count = mQueue.poll_count();
        XVERBOSEF("smtl", "queue task (%d): event count %lu", mQueue.get_fd(), count);
        LOGLET_XINDENT_SCOPE("smtl", loglet::Level::Verbose);

        for (uint64_t i = 0; i < count; i++) {
            auto data = mQueue.pop();
            for (auto& inspector : mInspectors) {
                inspector->inspect(mSystem, data);
            }

            auto it = mConsumers.begin();
            while (it != mConsumers.end()) {
                auto consumer = it->get();
                if (std::next(it) != mConsumers.end()) {
                    auto clone = streamline::Clone<T>{}(data);
                    XVERBOSEF("smtl", "cloning data for next consumer");
                    consumer->consume(mSystem, std::move(clone));
                } else {
                    consumer->consume(mSystem, std::move(data));
                }
                it++;
            }
        }
    }

    void push(T&& value) { mQueue.push(std::move(value)); }

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
    EventQueue<T>                              mQueue;
    std::vector<std::unique_ptr<Consumer<T>>>  mConsumers;
    std::vector<std::unique_ptr<Inspector<T>>> mInspectors;
};

}  // namespace streamline
