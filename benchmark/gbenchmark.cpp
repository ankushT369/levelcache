#include "benchmark/benchmark.h"
#include <unistd.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

extern "C" {
#include "splitcache.h"
}

const char* DB_PATH_BENCH = "/tmp/splitcache_gbenchmark_db";

// Helper to generate random strings
static void generate_random_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof(charset) - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
}

class SplitCacheBenchmark : public benchmark::Fixture {
public:
    static SplitCache* cache;
    static std::vector<std::string> keys;
};

SplitCache* SplitCacheBenchmark::cache = nullptr;
std::vector<std::string> SplitCacheBenchmark::keys;


BENCHMARK_F(SplitCacheBenchmark, BM_Write)(benchmark::State& state) {
    char key[32];
    char value[128];
    for (auto _ : state) {
        generate_random_string(key, sizeof(key));
        generate_random_string(value, sizeof(value));
        if (splitcache_put(cache, key, value, 0) != 0) {
            state.SkipWithError("Put failed");
        }
    }
}

BENCHMARK_F(SplitCacheBenchmark, BM_Read)(benchmark::State& state) {
    if (keys.empty()) {
        state.SkipWithError("No keys to read");
        return;
    }
    for (auto _ : state) {
        const std::string& key = keys[rand() % keys.size()];
        char* val = splitcache_get(cache, key.c_str());
        if (val) {
            benchmark::DoNotOptimize(val);
            free(val);
        }
    }
}

// Custom main to control global setup and teardown
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    char command[256];
    snprintf(command, sizeof(command), "rm -rf %s", DB_PATH_BENCH);
    system(command);
    SplitCacheBenchmark::cache = splitcache_open(DB_PATH_BENCH, 100); // 100 MB cache
    if (!SplitCacheBenchmark::cache) {
        fprintf(stderr, "Failed to open database. Aborting benchmarks.\n");
        return 1;
    }

    printf("Preparing benchmark data...\n");
    SplitCacheBenchmark::keys.reserve(20000);
    for (int i = 0; i < 20000; ++i) {
        char key_buf[32];
        char val_buf[128];
        generate_random_string(key_buf, sizeof(key_buf));
        generate_random_string(val_buf, sizeof(val_buf));
        splitcache_put(SplitCacheBenchmark::cache, key_buf, val_buf, 0);
        SplitCacheBenchmark::keys.push_back(key_buf);
    }
    printf("Data preparation complete.\n");
    
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        splitcache_close(SplitCacheBenchmark::cache);
        snprintf(command, sizeof(command), "rm -rf %s", DB_PATH_BENCH);
        system(command);
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    splitcache_close(SplitCacheBenchmark::cache);
    snprintf(command, sizeof(command), "rm -rf %s", DB_PATH_BENCH);
    system(command);
    printf("Benchmark cleanup complete.\n");

    return 0;
}