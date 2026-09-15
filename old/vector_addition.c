// neon_add_bench.c

#include <arm_neon.h>
#include <mach/mach_time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------------------------------------------
// Scalar addition
// ------------------------------------------------------------

__attribute__((noinline))
void scalar_add(const float *a,
                const float *b,
                float *c,
                size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ------------------------------------------------------------
// NEON addition
//
// float32x4_t = 4 x float32 = 128 bits
// vld1q_f32  -> load 4 floats
// vaddq_f32  -> add 4 floats
// vst1q_f32  -> store 4 floats
// ------------------------------------------------------------

__attribute__((noinline))
void neon_add(const float *a,
              const float *b,
              float *c,
              size_t n)
{
    size_t i = 0;

    // Process 4 floats per iteration.
    for (; i + 4 <= n; i += 4) {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);

        float32x4_t vc = vaddq_f32(va, vb);

        vst1q_f32(c + i, vc);
    }

    // Handle remainder.
    for (; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ------------------------------------------------------------
// ARM virtual counter
//
// CNTVCT_EL0 is available from userspace on macOS ARM64.
// IMPORTANT: these are timer ticks, NOT CPU cycles.
// ------------------------------------------------------------

static inline uint64_t read_cntvct(void)
{
    uint64_t value;

    asm volatile(
        "isb\n"
        "mrs %0, cntvct_el0\n"
        : "=r"(value)
        :
        : "memory"
    );

    return value;
}

static inline uint64_t read_cntfrq(void)
{
    uint64_t value;

    asm volatile(
        "mrs %0, cntfrq_el0\n"
        : "=r"(value)
    );

    return value;
}

// ------------------------------------------------------------
// mach_absolute_time -> nanoseconds
// ------------------------------------------------------------

static mach_timebase_info_data_t timebase;

static inline uint64_t mach_time_to_ns(uint64_t t)
{
    return t * timebase.numer / timebase.denom;
}

// ------------------------------------------------------------
// Prevent compiler from optimizing away the result.
// ------------------------------------------------------------

static volatile float checksum = 0.0f;

static void consume_result(const float *c, size_t n)
{
    float sum = 0.0f;

    // Sample periodically rather than doing another expensive
    // full traversal.
    for (size_t i = 0; i < n; i += 4096) {
        sum += c[i];
    }

    checksum += sum;
}

// ------------------------------------------------------------
// Benchmark helper
// ------------------------------------------------------------

typedef void (*add_fn)(const float *, const float *, float *, size_t);

static void benchmark(const char *name,
                      add_fn fn,
                      const float *a,
                      const float *b,
                      float *c,
                      size_t n,
                      int iterations)
{
    // Warm up.
    for (int i = 0; i < 10; ++i) {
        fn(a, b, c, n);
        consume_result(c, n);
    }

    uint64_t total_cntvct = 0;
    uint64_t total_ns = 0;

    uint64_t min_cntvct = UINT64_MAX;
    uint64_t min_ns = UINT64_MAX;

    for (int i = 0; i < iterations; ++i) {

        // CNTVCT measurement
        uint64_t start_cntvct = read_cntvct();

        // Also measure with macOS monotonic clock.
        uint64_t start_ns = mach_time_to_ns(mach_absolute_time());

        fn(a, b, c, n);

        consume_result(c, n);

        uint64_t end_ns = mach_time_to_ns(mach_absolute_time());
        uint64_t end_cntvct = read_cntvct();

        uint64_t cntvct_delta = end_cntvct - start_cntvct;
        uint64_t ns_delta = end_ns - start_ns;

        total_cntvct += cntvct_delta;
        total_ns += ns_delta;

        if (cntvct_delta < min_cntvct)
            min_cntvct = cntvct_delta;

        if (ns_delta < min_ns)
            min_ns = ns_delta;
    }

    double avg_cntvct =
        (double)total_cntvct / iterations;

    double avg_ns =
        (double)total_ns / iterations;

    double elements_per_second =
        ((double)n * iterations) /
        ((double)total_ns / 1e9);

    double ns_per_element =
        avg_ns / (double)n;

    printf("\n%s\n", name);
    printf("----------------------------------------\n");
    printf("Average CNTVCT ticks : %.2f\n", avg_cntvct);
    printf("Minimum CNTVCT ticks : %llu\n",
           (unsigned long long)min_cntvct);

    printf("Average time         : %.2f ns\n", avg_ns);
    printf("Minimum time         : %llu ns\n",
           (unsigned long long)min_ns);

    printf("ns / element         : %.6f\n", ns_per_element);
    printf("Elements / second    : %.3f billion\n",
           elements_per_second / 1e9);
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main(void)
{
    const size_t N = 1024 * 1024;   // 1M floats
    const int ITERATIONS = 1000;

    mach_timebase_info(&timebase);

    uint64_t cntfrq = read_cntfrq();

    printf("========================================\n");
    printf(" Apple Silicon NEON Vector Addition\n");
    printf("========================================\n");

    printf("Elements             : %zu\n", N);
    printf("Data per array       : %.2f MB\n",
           (double)(N * sizeof(float)) / (1024.0 * 1024.0));

    printf("CNTVCT frequency     : %llu Hz\n",
           (unsigned long long)cntfrq);

    printf("\nNOTE:\n");
    printf("CNTVCT ticks are NOT CPU cycles.\n");
    printf("Use Instruments CPU Counters/Processor Trace\n");
    printf("for actual CPU-cycle measurements.\n");

    // 64-byte alignment is useful for cache-line alignment.
    float *a = NULL;
    float *b = NULL;
    float *c = NULL;

    if (posix_memalign((void **)&a, 64, N * sizeof(float)) != 0 ||
        posix_memalign((void **)&b, 64, N * sizeof(float)) != 0 ||
        posix_memalign((void **)&c, 64, N * sizeof(float)) != 0) {

        fprintf(stderr, "Allocation failed\n");

        free(a);
        free(b);
        free(c);

        return 1;
    }

    // Initialize.
    for (size_t i = 0; i < N; ++i) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
    }

    // Benchmark scalar.
    benchmark(
        "Scalar addition",
        scalar_add,
        a,
        b,
        c,
        N,
        ITERATIONS
    );

    // Benchmark NEON.
    benchmark(
        "NEON addition",
        neon_add,
        a,
        b,
        c,
        N,
        ITERATIONS
    );

    printf("\nChecksum: %f\n", checksum);

    free(a);
    free(b);
    free(c);

    return 0;
}