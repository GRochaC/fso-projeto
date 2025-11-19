#include "stdlib.h"

// Processo que ficara ocupando a CPU

int main() {
    unsigned long sum = 0;
    for (unsigned long i = 0; i < 40 * 1e8; ++i) {
        sum += i;
    }
    exit(0);
}