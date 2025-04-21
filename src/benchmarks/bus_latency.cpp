#include <benchmark/benchmark.h>
#include "algotrading/core/EventBus.hpp"
#include "algotrading/events/MarketDataEvent.hpp"
#include <atomic>

using namespace algotrading;
// Helper: single producer publishes N events to a 1‑worker EventBus.
// Throughput is measured in events / second.

class EventBusFixture : public benchmark::Fixture {
protected:
    core::EventBus bus;
    std::atomic<int> handled{0};

    void SetUp(const ::benchmark::State&) override {
        // Subscribe a trivial handler so events get consumed.
        bus.subscribe<events::MarketDataEvent>([&](const events::MarketDataEvent&) {
            ++handled;
        });
        bus.start(1); // 1 worker thread for a clear baseline
    }

    void TearDown(const ::benchmark::State&) override {
        bus.stop();
    }
};

BENCHMARK_F(EventBusFixture, PublishMarketData)(benchmark::State& state) {
events::MarketDataEvent ev;
ev.symbol = "AAPL";
ev.price  = 123.45;
ev.volume = 1000;
ev.timestamp_ns = 0;

for (auto _ : state) {
bus.publish(ev);
}

// Let any remaining events drain before reporting counters
bus.stop();
state.counters["events_processed"] = handled.load();
}

// Register with multiple thread counts to see scaling behaviour
BENCHMARK_REGISTER_F(EventBusFixture, PublishMarketData)
->Threads(1)
->Threads(2)
->Threads(4)
->Threads(8);

BENCHMARK_MAIN();