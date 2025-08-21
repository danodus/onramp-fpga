#include "io.h"

#define LED 0x20000000

void set_led(int value) {
    *(uint32_t *)LED = value;
}