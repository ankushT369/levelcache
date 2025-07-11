#include "gtest/gtest.h"
#include <unistd.h>

extern "C" {
#include "levelcache.h"
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
        cache = levelcache_open(DB_PATH, 0, 0);
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
    cache = levelcache_open(DB_PATH, 0, 2); // 2 seconds default TTL
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

} // namespace