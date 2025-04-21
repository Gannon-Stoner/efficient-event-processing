#pragma once
#include <cstdint>
#include <string>

namespace algotrading::events {

    enum class Side { Buy, Sell };

    struct OrderEvent {
        std::string order_id;
        std::string symbol;
        Side side{Side::Buy};
        double price{0.0};
        double quantity{0.0};
        std::uint64_t timestamp_ns{0};
    };

}
