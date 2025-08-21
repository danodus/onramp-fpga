#include "io.h"

int main(int argc, char *argv[]) {
    int i = 0;

    for (;;) {
        set_led(i >> 16);
        i++;
    }

    return 0;
}