#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

// Simple checksum so we can confirm every kernel produces the SAME result
// as we optimize. If two methods print different sums, the optimization is wrong.
static uint64_t checksum(int P, int R, const int *C) {
    uint64_t s = 0;
    for (int i = 0; i < P * R; i++) s += (uint64_t)C[i];
    return s;
}

static void run(mat_args *obj, int type, const char *name) {
    // Reset C so results don't accumulate across methods (correctness).
    memset(obj->C, 0, (size_t)obj->P * obj->R * sizeof(int));
    uint64_t start = get_time();
    iterator(10, obj, type);
    uint64_t end = get_time();
    printf("%-18s duration: %9"PRIu64" us   checksum: %"PRIu64"\n",
           name, (end - start)/1000, checksum(obj->P, obj->R, obj->C));
}

int main() {
    mat_args obj;
    obj.P = 1024;
    obj.Q = 1024;
    obj.R = 1024;
    obj.A = init_mat(obj.P, obj.Q, SUM);
    obj.B = init_mat(obj.Q, obj.R, PRODUCT);
    obj.C = init_mat(obj.P, obj.R, ZERO);

    // printf("Printing mat A\n");
    // print_mat(obj.P, obj.Q, obj.A);
    // printf("\n\n");
    // printf("Pringing mat B\n");
    // print_mat(obj.Q, obj.R, obj.B);
    // printf("\n\n");

    // Warm-up: first pass pays page-fault / cold-cache cost that would
    // otherwise be unfairly charged to the STANDARD kernel.
    run(&obj, ITERATION_REORDER, "warmup");
    // printf("Printing mat C\n");
    // print_mat(obj.P, obj.R, obj.C);
    // printf("\n\n");

    run(&obj, STANDARD,          "standard");
    // printf("Printing mat C\n");
    // print_mat(obj.P, obj.R, obj.C);
    // printf("\n\n");
    run(&obj, ITERATION_REORDER, "iteration_reorder");
    run(&obj, CACHE_TILING,      "cache_tiling");
    run(&obj, CACHE_TILING_IJK,  "cache_tiling_ijk");
    run(&obj, MULTI_THREAD,      "multi_thread");
    run(&obj, REGISTER_TILING,   "register_tiling");
    // printf("Printing mat C\n");
    // print_mat(obj.P, obj.R, obj.C);
    // printf("\n\n");
    run(&obj, REGISTER_TILING_MT, "register_tiling_mt");

    free(obj.A);
    free(obj.B);
    free(obj.C);
}
