#include <algotrading/core/EventBus.hpp>
#include <utility>

namespace algotrading::core {

    EventBus::EventBus() = default;
    EventBus::~EventBus() { stop(); }

    void EventBus::start(std::size_t workers) {
        if (running_.exchange(true)) return;
        for (std::size_t i = 0; i < workers; ++i)
            workers_.emplace_back(&EventBus::worker_loop, this);
    }

    void EventBus::stop() {
        if (!running_.exchange(false)) return;
        cv_.notify_all();
        for (auto& t : workers_) if (t.joinable()) t.join();
        workers_.clear();
    }

    void EventBus::drain_latency(std::vector<uint32_t>& out) {
        std::lock_guard<std::mutex> lk(lat_mtx_);
        out.swap(latency_ns_);
    }

    void EventBus::worker_loop() {
        while (running_) {
            Prioritized<TimedEvent> item;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [&] { return !queue_.empty() || !running_; });
                if (!running_) break;
                item = std::move(const_cast<Prioritized<TimedEvent>&>(queue_.top()));
                queue_.pop();
            }

            auto& timed = item.data;
            if (timed.enqueue_ns != 0) {
                uint32_t dur = static_cast<uint32_t>(
                        std::chrono::steady_clock::now().time_since_epoch().count() - timed.enqueue_ns);
                std::lock_guard<std::mutex> lk(lat_mtx_);
                if (latency_ns_.size() < 1'000'000) latency_ns_.push_back(dur);
            }

            std::visit([&](auto&& concrete) {
                using T = std::decay_t<decltype(concrete)>;
                auto it = handlers_.find(std::type_index(typeid(T)));
                if (it == handlers_.end()) return;
                for (auto& e : it->second) {
                    if (!e.filter || e.filter(timed.ev)) e.handler(timed.ev);
                }
            }, timed.ev);
        }
    }

}
