// cache_latency.c
//
// Measures approximate memory-access latency as the working-set size grows.
// Designed for macOS (Apple Silicon / Intel).
//
// Compile:
//     clang -O2 -o cache_latency cache_latency.c
//
// Run:
//     ./cache_latency

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MIN_SIZE_KB 4
#define MAX_SIZE_MB 256

// Number of dependent pointer-chasing operations.
// Increase this if measurements are too noisy.
#define ITERATIONS 10000000ULL

static inline uint64_t get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL +
           (uint64_t)ts.tv_nsec;
}

// Shuffle an array so that accesses are unpredictable.
// We create a circular linked list:
//
// 0 -> 7 -> 2 -> 5 -> ... -> 0
//
// This prevents the hardware prefetcher from easily predicting
// the next address.
static void create_random_cycle(size_t *array, size_t n)
{
    // Create indices
    for (size_t i = 0; i < n; i++)
        array[i] = i;

    // Fisher-Yates shuffle
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);

        size_t temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }

    // Convert shuffled indices into a cycle.
    //
    // array[0] -> array[1] -> array[2] -> ...
    // -> array[n-1] -> array[0]
    //
    // We store the NEXT INDEX in each location.
    size_t *next = malloc(n * sizeof(size_t));

    if (!next) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < n - 1; i++)
        next[array[i]] = array[i + 1];

    next[array[n - 1]] = array[0];

    for (size_t i = 0; i < n; i++)
        array[i] = next[i];

    free(next);
}

// Perform dependent pointer chasing.
//
// The important part is:
//
//     index = array[index];
//
// The next memory address cannot be known until the previous
// memory access completes.
static size_t pointer_chase(
    volatile size_t *array,
    size_t index,
    uint64_t iterations)
{
    for (uint64_t i = 0; i < iterations; i++) {
        index = array[index];
    }

    return index;
}

int main(void)
{
    printf("Cache / Memory Latency Benchmark\n");
    printf("---------------------------------\n");
    printf("Iterations per test: %llu\n\n",
           (unsigned long long)ITERATIONS);

    // Prevent rand() from producing the same pattern every run.
    srand(12345);

    printf("%12s %15s %15s\n",
           "Working Set", "Latency (ns)", "Latency (cycles)");

    printf("%12s %15s %15s\n",
           "-----------", "------------", "----------------");

    for (size_t size_kb = MIN_SIZE_KB;
         size_kb <= MAX_SIZE_MB * 1024;
         size_kb *= 2) {

        size_t bytes = size_kb * 1024;

        // Number of size_t elements.
        size_t n = bytes / sizeof(size_t);

        size_t *array = aligned_alloc(64, n * sizeof(size_t));

        if (!array) {
            perror("aligned_alloc");
            return EXIT_FAILURE;
        }

        create_random_cycle(array, n);

        // Warm up the cache / memory system.
        size_t index = 0;

        index = pointer_chase(
            array,
            index,
            100000
        );

        // Measure.
        uint64_t start = get_time_ns();

        index = pointer_chase(
            array,
            index,
            ITERATIONS
        );

        uint64_t end = get_time_ns();

        uint64_t elapsed_ns = end - start;

        double ns_per_access =
            (double)elapsed_ns / ITERATIONS;

        // Approximate CPU frequency.
        //
        // We don't know the actual CPU frequency here,
        // especially because modern CPUs dynamically change
        // frequency. Therefore cycles are only an approximation.
        //
        // You can change this value based on your CPU.
        double assumed_ghz = 3.0;

        double cycles =
            ns_per_access * assumed_ghz;

        printf("%9zu KB %15.3f %15.2f\n",
               size_kb,
               ns_per_access,
               cycles);

        // Prevent compiler from completely eliminating the work.
        if (index == (size_t)-1)
            printf("Impossible: %zu\n", index);

        free(array);
    }

    return 0;
}
