#pragma once
#include <variant>
#include <algotrading/events/MarketDataEvent.hpp>
#include <algotrading/events/OrderEvent.hpp>
#include <algotrading/events/SystemEvent.hpp>

namespace algotrading {

    using Event = std::variant<
            events::MarketDataEvent,
            events::OrderEvent,
            events::SystemEvent
    >;

}
