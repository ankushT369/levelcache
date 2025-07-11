#include "gtest/gtest.h"
#include <unistd.h>

extern "C" {
#include "levelcache.h"
#include "log.h"
}

namespace {

const char* DB_PATH = "/tmp/levelcache_test_db";

class LevelCacheTest : public ::testing::Test {
protected:
    LevelCache *cache;

    void SetUp() override {
        char command[256];
        snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
        system(command);
        cache = levelcache_open(DB_PATH, 0, 1, 0, LOG_FATAL);
        ASSERT_NE(cache, nullptr);
    }

    void TearDown() override {
        levelcache_close(cache);
        char command[256];
        snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
        system(command);
    }
};

TEST_F(LevelCacheTest, PutAndGet) {
    const char *key = "test_key";
    const char *value = "test_value";

    ASSERT_EQ(levelcache_put(cache, key, value, 0), 0);

    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);

    free(retrieved_value);
}

TEST_F(LevelCacheTest, GetNonExistent) {
    const char *key = "non_existent_key";
    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(LevelCacheTest, Delete) {
    const char *key = "test_key";
    const char *value = "test_value";

    ASSERT_EQ(levelcache_put(cache, key, value, 0), 0);
    ASSERT_EQ(levelcache_delete(cache, key), 0);

    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(LevelCacheTest, Ttl) {
    const char *key = "ttl_key";
    const char *value = "ttl_value";

    // Test that the key expires after 1 second
    ASSERT_EQ(levelcache_put(cache, key, value, 1), 0);
    sleep(2);
    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);

    // Test that the key does not expire before the TTL
    ASSERT_EQ(levelcache_put(cache, key, value, 5), 0);
    retrieved_value = levelcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);
    free(retrieved_value);
}

TEST_F(LevelCacheTest, OverwriteKey) {
    const char *key = "overwrite_key";
    const char *value1 = "value1";
    const char *value2 = "value2";

    ASSERT_EQ(levelcache_put(cache, key, value1, 0), 0);
    char *retrieved_value = levelcache_get(cache, key);
    EXPECT_STREQ(retrieved_value, value1);
    free(retrieved_value);

    ASSERT_EQ(levelcache_put(cache, key, value2, 0), 0);
    retrieved_value = levelcache_get(cache, key);
    EXPECT_STREQ(retrieved_value, value2);
    free(retrieved_value);
}

TEST_F(LevelCacheTest, UpdateTtl) {
    const char *key = "update_ttl_key";
    const char *value = "update_ttl_value";

    // Put with a short TTL
    ASSERT_EQ(levelcache_put(cache, key, value, 1), 0);
    
    // Update with a longer TTL
    ASSERT_EQ(levelcache_put(cache, key, value, 3), 0);

    // Wait for the initial TTL to expire
    sleep(2);

    // The key should still exist
    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    free(retrieved_value);

    // Wait for the updated TTL to expire
    sleep(2);

    // The key should now be expired
    retrieved_value = levelcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(LevelCacheTest, DeleteNonExistent) {
    ASSERT_EQ(levelcache_delete(cache, "non_existent_key"), 0);
}

TEST_F(LevelCacheTest, EmptyValue) {
    const char *key = "empty_value_key";
    const char *value = "";

    ASSERT_EQ(levelcache_put(cache, key, value, 0), 0);
    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);
    free(retrieved_value);
}

TEST_F(LevelCacheTest, DefaultTtl) {
    levelcache_close(cache);
    cache = levelcache_open(DB_PATH, 0, 2, 0, LOG_FATAL); // 2 seconds default TTL
    ASSERT_NE(cache, nullptr);

    const char *key = "default_ttl_key";
    const char *value = "default_ttl_value";

    // Put with ttl_seconds = 0, so default TTL should be used
    ASSERT_EQ(levelcache_put(cache, key, value, 0), 0);
    
    char *retrieved_value = levelcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);
    free(retrieved_value);

    sleep(3);

    retrieved_value = levelcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(LevelCacheTest, CleanupThread) {
    levelcache_close(cache);
    cache = levelcache_open(DB_PATH, 0, 1, 1, LOG_FATAL); // 1 second cleanup frequency
    ASSERT_NE(cache, nullptr);

    const char *key1 = "key1";
    const char *value1 = "value1";
    ASSERT_EQ(levelcache_put(cache, key1, value1, 1), 0);

    const char *key2 = "key2";
    const char *value2 = "value2";
    ASSERT_EQ(levelcache_put(cache, key2, value2, 3), 0);

    // Wait for the cleanup thread to run
    sleep(3);

    // key1 should be gone
    char *retrieved_value = levelcache_get(cache, key1);
    ASSERT_EQ(retrieved_value, nullptr);

    // key2 should still be there
    retrieved_value = levelcache_get(cache, key2);
    ASSERT_NE(retrieved_value, nullptr);
    free(retrieved_value);

    // Wait for key2 to expire
    sleep(2);

    retrieved_value = levelcache_get(cache, key2);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(LevelCacheTest, LogLevel) {
    levelcache_close(cache);
    cache = levelcache_open(DB_PATH, 0, 1, 0, LOG_INFO);
    ASSERT_NE(cache, nullptr);
    ASSERT_EQ(cache->log_level, LOG_INFO);
}

TEST_F(LevelCacheTest, MemoryUsage) {
    size_t initial_memory = levelcache_get_memory_usage(cache);
    
    const char *key1 = "mem_key_1";
    const char *value1 = "value1";
    levelcache_put(cache, key1, value1, 0);
    size_t memory_after_put1 = levelcache_get_memory_usage(cache);
    ASSERT_GT(memory_after_put1, initial_memory);

    const char *key2 = "mem_key_2";
    const char *value2 = "value2";
    levelcache_put(cache, key2, value2, 0);
    size_t memory_after_put2 = levelcache_get_memory_usage(cache);
    ASSERT_GT(memory_after_put2, memory_after_put1);

    levelcache_delete(cache, key1);
    size_t memory_after_delete1 = levelcache_get_memory_usage(cache);
    ASSERT_LT(memory_after_delete1, memory_after_put2);
    
    levelcache_delete(cache, key2);
    size_t memory_after_delete2 = levelcache_get_memory_usage(cache);
    ASSERT_EQ(memory_after_delete2, initial_memory);
}

} // namespace