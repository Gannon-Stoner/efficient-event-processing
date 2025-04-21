#pragma once
#include <cstdint>
#include <string>

namespace algotrading::events {

    struct MarketDataEvent {
        std::string symbol;
        double price{0.0};
        double volume{0.0};
        std::uint64_t timestamp_ns{0};   // nanoseconds since epoch
    };

}