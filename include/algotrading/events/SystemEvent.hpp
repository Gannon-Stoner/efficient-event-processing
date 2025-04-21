#pragma once
#include <cstdint>
#include <string>

namespace algotrading::events {

    enum class Severity { Info, Warning, Error };

    struct SystemEvent {
        Severity severity{Severity::Info};
        std::string message;
        std::uint64_t timestamp_ns{0};
    };

}
