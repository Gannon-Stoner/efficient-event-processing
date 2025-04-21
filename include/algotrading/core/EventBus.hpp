#pragma once

#include <algotrading/core/Event.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <chrono>

namespace algotrading::core {

// Priority
    enum class Priority : std::uint8_t { High = 0, Normal = 1, Low = 2 };

    template<typename T>
    struct Prioritized {
        Priority pr;
        T        data;
        // std::priority_queue keeps the *largest* on top, so invert comparison
        bool operator<(const Prioritized& other) const {
            return static_cast<int>(pr) > static_cast<int>(other.pr);
        }
    };

// Time‑stamped wrapper
    struct TimedEvent {
        algotrading::Event ev;
        std::uint64_t enqueue_ns{0};
    };

    class EventBus {
    public:
        EventBus();
        ~EventBus();

        EventBus(const EventBus&)            = delete;
        EventBus& operator=(const EventBus&) = delete;
        EventBus(EventBus&&)                 = delete;
        EventBus& operator=(EventBus&&)      = delete;

        template<typename EventT>
        using Handler = std::function<void(const EventT&)>;

        template<typename EventT>
        using Filter  = std::function<bool(const EventT&)>; // optional predicate

        // Subscribe with optional filter
        template<typename EventT>
        void subscribe(Handler<EventT> handler, Filter<EventT> filter = nullptr);

        // Publish event with priority
        template<typename EventT>
        void publish(EventT&& ev, Priority pr = Priority::Normal);

        void start(std::size_t workers = std::thread::hardware_concurrency());
        void stop();

        // Extract and clear collected latency samples (nanoseconds)
        void drain_latency(std::vector<uint32_t>& out);

    private:
        struct HandlerEntry {
            std::function<void(const algotrading::Event&)> handler;
            std::function<bool(const algotrading::Event&)> filter; // may be empty
        };

        void worker_loop();

        // state
        std::mutex mtx_;
        std::condition_variable cv_;
        std::priority_queue<Prioritized<TimedEvent>> queue_;

        std::unordered_map<std::type_index, std::vector<HandlerEntry>> handlers_;
        std::vector<std::thread> workers_;
        std::atomic<bool> running_{false};

        // latency collection
        std::mutex lat_mtx_;
        std::vector<uint32_t> latency_ns_;
    };

// template implementations

    template<typename EventT>
    void EventBus::subscribe(Handler<EventT> handler, Filter<EventT> filter) {
        std::lock_guard<std::mutex> lk(mtx_);
        auto& vec = handlers_[std::type_index(typeid(EventT))];
        vec.push_back({
                              [h = std::move(handler)](const algotrading::Event& base) {
                                  h(std::get<EventT>(base));
                              },
                              filter ? [f = std::move(filter)](const algotrading::Event& base) {
                                  return f(std::get<EventT>(base));
                              } : std::function<bool(const algotrading::Event&)>()
                      });
    }

    template<typename EventT>
    void EventBus::publish(EventT&& ev, Priority pr) {
        TimedEvent node;
        node.ev = std::forward<EventT>(ev);
        node.enqueue_ns = std::chrono::steady_clock::now().time_since_epoch().count();

        {
            std::lock_guard<std::mutex> lk(mtx_);
            queue_.push(Prioritized<TimedEvent>{pr, std::move(node)});
        }
        cv_.notify_one();
    }

}