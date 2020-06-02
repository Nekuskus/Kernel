#pragma once
#include <stdint.h>

namespace E820 {
    enum class [[gnu::packed]] E820Type : uint32_t{
        USABLE_RAM = 1,
        RESERVED,
        ACPI_RECLAIMABLE,
        ACPI_NVS,
        BAD_MEMORY,
    };

    struct [[gnu::packed]] E820Entry {
        uint64_t base;
        uint64_t length;
        E820Type type;
        uint32_t unused;
    };
}  // namespace E820