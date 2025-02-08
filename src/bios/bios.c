#define LED 0x20000000

typedef unsigned int uint32_t;

int main(int argc, char *argv[]) {
    int i = 0;

    for (;;) {
        *(uint32_t *)LED = i >> 16;
        i++;
    }

    return 0;
}