#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>


#define ITERATIONS 100000000

double get_time_elapsed(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    struct timespec start, end;
    double result = 0.0;

    // Chronométrage du début
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < ITERATIONS; i++) {
        result += sin(i) * cos(i);  // Charge de calcul
    }

    // Chronométrage de la fin
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcul du temps écoulé
    double elapsed_time = get_time_elapsed(start, end);

    printf("Temps écoulé : %.2f secondes\n", elapsed_time);
    printf("Résultat final (pour éviter les optimisations) : %f\n", result);

    return 0;
}

