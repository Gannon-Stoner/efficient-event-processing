#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <algotrading/core/EventBus.hpp>
#include <algotrading/events/MarketDataEvent.hpp>
#include <algotrading/events/OrderEvent.hpp>
#include <algotrading/events/SystemEvent.hpp>
#include <atomic>
#include <thread>

using namespace algotrading;

TEST_CASE("MarketDataEvent publish/subscribe", "[eventbus]") {
core::EventBus bus;

std::atomic<bool> handled{false};
bus.subscribe<events::MarketDataEvent>([&](const events::MarketDataEvent& ev) {
REQUIRE(ev.symbol == "AAPL");
REQUIRE(ev.price == Catch::Approx(123.45));
handled = true;
});

bus.start(1); // single worker

events::MarketDataEvent md;
md.symbol = "AAPL";
md.price = 123.45;
md.volume = 1000;
md.timestamp_ns = 42;
bus.publish(md);

// give worker thread a moment
std::this_thread::sleep_for(std::chrono::milliseconds(10));
bus.stop();

REQUIRE(handled.load());
}

TEST_CASE("Multiple event types handled", "[eventbus]") {
core::EventBus bus;
std::atomic<int> count{0};

bus.subscribe<events::OrderEvent>([&](const events::OrderEvent&) {
++count;
});
bus.subscribe<events::SystemEvent>([&](const events::SystemEvent&) {
++count;
});

bus.start(2); // two worker threads

events::OrderEvent oe;
oe.order_id = "42";
oe.symbol = "ES";
bus.publish(oe);

events::SystemEvent se;
se.severity = events::Severity::Info;
se.message = "Service start";
bus.publish(se);

std::this_thread::sleep_for(std::chrono::milliseconds(20));
bus.stop();

REQUIRE(count.load() == 2);
}