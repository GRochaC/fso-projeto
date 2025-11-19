#include "stdlib.h"

// Processo que ficará ocupando a CPU

int main() {
    unsigned long sum = 0;
    for (unsigned long i = 0; i < 4 * 1e9; ++i) {
        sum += i;
    }
    exit(0);
}
