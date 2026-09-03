/*
 * corebench.c — deterministic multi-threaded CPU exercise utility
 *
 * Spawns N worker threads, each computes a rolling checksum over a
 * pseudo-random (but seeded, reproducible) stream of 64-bit words.
 * Workers report per-thread checksums; the program exits 0 iff every
 * thread's checksum matches the reference vector computed by the same
 * algorithm single-threaded at startup.
 *
 * No external dependencies. C11, pthreads only.
 */

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_THREADS   4
#define DEFAULT_ITERS     100000
#define FNV64_OFFSET      1469598103934665603ULL
#define FNV64_PRIME       1099511628211ULL

typedef struct {
    uint32_t seed;
    uint64_t iterations;
    uint64_t checksum;
    int      ok;
} worker_arg;

/* xorshift64* PRNG — reproducible across platforms */
static uint64_t xs64(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

/* single-threaded reference for one (seed, iterations) pair */
static uint64_t reference_checksum(uint32_t seed, uint64_t iterations)
{
    uint64_t state = seed ? seed : 1;
    uint64_t h = FNV64_OFFSET;
    for (uint64_t i = 0; i < iterations; i++)
        h = (h ^ xs64(&state)) * FNV64_PRIME;
    return h;
}

static void *worker(void *p)
{
    worker_arg *a = (worker_arg *)p;
    a->checksum = reference_checksum(a->seed, a->iterations);
    a->ok = 1;
    return NULL;
}

static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, char **argv)
{
    int      threads    = DEFAULT_THREADS;
    uint64_t iterations = DEFAULT_ITERS;
    int      json_mode  = 0;
    int      quiet      = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--threads") && i + 1 < argc)
            threads = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--iterations") && i + 1 < argc)
            iterations = strtoull(argv[++i], NULL, 10);
        else if (!strcmp(argv[i], "--json"))
            json_mode = 1;
        else if (!strcmp(argv[i], "--quiet"))
            quiet = 1;
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            printf("usage: corebench [--threads N] [--iterations N] [--json] [--quiet]\n");
            return 0;
        }
    }

    if (threads < 1)  threads = 1;
    if (threads > 256) threads = 256;

    pthread_t  tid[256];
    worker_arg args[256];
    double t0 = now_s();

    for (int i = 0; i < threads; i++) {
        args[i].seed       = 0x9E3779B9u + (uint32_t)i;
        args[i].iterations = iterations;
        args[i].ok         = 0;
        if (pthread_create(&tid[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "corebench: pthread_create failed\n");
            return 2;
        }
    }

    int all_ok = 1;
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
        if (!args[i].ok || args[i].checksum != reference_checksum(args[i].seed, iterations))
            all_ok = 0;
    }

    double elapsed = now_s() - t0;

    if (json_mode) {
        printf("{\"tool\":\"corebench\",\"threads\":%d,\"iterations\":%llu,"
               "\"elapsed_s\":%.2f,\"checksum\":\"%llx\"}\n",
               threads, (unsigned long long)iterations, elapsed,
               (unsigned long long)(all_ok ? args[0].checksum : 0));
    } else if (!quiet) {
        printf("corebench: %d threads x %llu iters in %.2fs -> %s\n",
               threads, (unsigned long long)iterations, elapsed,
               all_ok ? "PASS" : "FAIL");
    }

    return all_ok ? 0 : 1;
}
