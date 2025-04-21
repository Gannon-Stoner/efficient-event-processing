#include <benchmark/benchmark.h>
#include <algotrading/core/EventBus.hpp>
#include <algotrading/events/MarketDataEvent.hpp>
#include <fstream>
#include <atomic>
#include <vector>

using namespace algotrading;

class LatencyFixture : public benchmark::Fixture {
protected:
    core::EventBus bus;
    std::atomic<int> processed{0};

    void SetUp(const benchmark::State&) override {
        bus.subscribe<events::MarketDataEvent>([&](const events::MarketDataEvent&) {
            ++processed;
        });
        bus.start(1);
    }

    void TearDown(const benchmark::State& state) override {
        bus.stop();
        std::vector<uint32_t> samples;
        bus.drain_latency(samples);
        std::ofstream csv("latency_" + std::to_string(state.threads()) + ".csv");
        for (auto v : samples) csv << v << '\n';
    }
};

BENCHMARK_DEFINE_F(LatencyFixture, PublishLatency)(benchmark::State& state) {
    events::MarketDataEvent ev{"AAPL", 123.45, 1000, 0};
    for (auto _ : state) {
        bus.publish(ev, core::Priority::Normal);
    }
}

BENCHMARK_REGISTER_F(LatencyFixture, PublishLatency)
        ->Threads(1)
        ->Threads(2)
        ->Threads(4);

BENCHMARK_MAIN();
