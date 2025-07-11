#include "benchmark/benchmark.h"
#include <unistd.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <numeric>

extern "C" {
#include "levelcache.h"
}

const char* DB_PATH_BENCH = "/tmp/levelcache_gbenchmark_db";

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

class LevelCacheBenchmark : public benchmark::Fixture {
public:
    LevelCache* cache = nullptr;
    std::vector<std::string> keys;

    void SetUp(const ::benchmark::State& state) override {
        if (cache == nullptr) {
            srand((unsigned int)time(NULL));
            char command[256];
            snprintf(command, sizeof(command), "rm -rf %s", DB_PATH_BENCH);
            system(command);
            cache = levelcache_open(DB_PATH_BENCH, 100, 0, 0, LOG_FATAL); // 100 MB cache
            if (!cache) {
                // This is a fatal error for the benchmark suite.
                // We use a raw fprintf and exit because this setup is outside the benchmark run state.
                fprintf(stderr, "Failed to open database. Aborting benchmarks.\n");
                exit(1);
            }

            // Pre-populate with data for read benchmarks
            keys.reserve(20000);
            for (int i = 0; i < 20000; ++i) {
                char key_buf[32];
                char val_buf[128];
                generate_random_string(key_buf, sizeof(key_buf));
                generate_random_string(val_buf, sizeof(val_buf));
                levelcache_put(cache, key_buf, val_buf, 0);
                keys.push_back(key_buf);
            }
        }
    }

    void TearDown(const ::benchmark::State& state) override {
        if (state.thread_index() == 0 && cache != nullptr) {
            levelcache_close(cache);
            cache = nullptr;
            char command[256];
            snprintf(command, sizeof(command), "rm -rf %s", DB_PATH_BENCH);
            system(command);
        }
    }
};

BENCHMARK_F(LevelCacheBenchmark, BM_Write)(benchmark::State& state) {
    char key[32];
    char value[128];
    std::vector<double> latencies;
    latencies.reserve(state.max_iterations);

    for (auto _ : state) {
        generate_random_string(key, sizeof(key));
        generate_random_string(value, sizeof(value));
        
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        if (levelcache_put(cache, key, value, 0) != 0) {
            state.SkipWithError("Put failed");
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        latencies.push_back((end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec));
    }

    std::sort(latencies.begin(), latencies.end());
    state.counters["p50(ns)"] = latencies[latencies.size() * 0.50];
    state.counters["p90(ns)"] = latencies[latencies.size() * 0.90];
    state.counters["p95(ns)"] = latencies[latencies.size() * 0.95];
    state.counters["p99(ns)"] = latencies[latencies.size() * 0.99];
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(LevelCacheBenchmark, BM_Read)(benchmark::State& state) {
    if (keys.empty()) {
        state.SkipWithError("No keys to read");
        return;
    }
    std::vector<double> latencies;
    latencies.reserve(state.max_iterations);

    for (auto _ : state) {
        const std::string& key = keys[rand() % keys.size()];
        
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        char* val = levelcache_get(cache, key.c_str());
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        latencies.push_back((end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec));

        if (val) {
            benchmark::DoNotOptimize(val);
            free(val);
        }
    }

    std::sort(latencies.begin(), latencies.end());
    state.counters["p50(ns)"] = latencies[latencies.size() * 0.50];
    state.counters["p90(ns)"] = latencies[latencies.size() * 0.90];
    state.counters["p95(ns)"] = latencies[latencies.size() * 0.95];
    state.counters["p99(ns)"] = latencies[latencies.size() * 0.99];
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_MAIN();