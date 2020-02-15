#pragma once
#include <stdint.h>

class Serial {
    uint16_t base;
    bool transmit_empty();
    bool received();

public:
    Serial(uint16_t com_port, uint32_t baud_rate);
    ~Serial();
    void set_baud_rate(uint32_t baud_rate);
    void write(const char a);
    void write(const char* a);
    char read();
};