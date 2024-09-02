#include <stdlib.h>
#include <time.h>

char randchar() {
    // Seed the random number generator only once
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    // Generate a random character from 'A' to 'Z'
    return 'A' + (rand() % 26);
}
