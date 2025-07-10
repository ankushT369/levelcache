#include "gtest/gtest.h"
#include <unistd.h>

extern "C" {
#include "splitcache.h"
}

namespace {

const char* DB_PATH = "/tmp/splitcache_test_db";

class SplitCacheTest : public ::testing::Test {
protected:
    SplitCache *cache;

    void SetUp() override {
        char command[256];
        snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
        system(command);
        cache = splitcache_open(DB_PATH, 0);
        ASSERT_NE(cache, nullptr);
    }

    void TearDown() override {
        splitcache_close(cache);
        char command[256];
        snprintf(command, sizeof(command), "rm -rf %s", DB_PATH);
        system(command);
    }
};

TEST_F(SplitCacheTest, PutAndGet) {
    const char *key = "test_key";
    const char *value = "test_value";

    ASSERT_EQ(splitcache_put(cache, key, value, 0), 0);

    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);

    free(retrieved_value);
}

TEST_F(SplitCacheTest, GetNonExistent) {
    const char *key = "non_existent_key";
    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(SplitCacheTest, Delete) {
    const char *key = "test_key";
    const char *value = "test_value";

    ASSERT_EQ(splitcache_put(cache, key, value, 0), 0);
    ASSERT_EQ(splitcache_delete(cache, key), 0);

    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(SplitCacheTest, Ttl) {
    const char *key = "ttl_key";
    const char *value = "ttl_value";

    // Test that the key expires after 1 second
    ASSERT_EQ(splitcache_put(cache, key, value, 1), 0);
    sleep(2);
    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);

    // Test that the key does not expire before the TTL
    ASSERT_EQ(splitcache_put(cache, key, value, 5), 0);
    retrieved_value = splitcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);
    free(retrieved_value);
}

TEST_F(SplitCacheTest, OverwriteKey) {
    const char *key = "overwrite_key";
    const char *value1 = "value1";
    const char *value2 = "value2";

    ASSERT_EQ(splitcache_put(cache, key, value1, 0), 0);
    char *retrieved_value = splitcache_get(cache, key);
    EXPECT_STREQ(retrieved_value, value1);
    free(retrieved_value);

    ASSERT_EQ(splitcache_put(cache, key, value2, 0), 0);
    retrieved_value = splitcache_get(cache, key);
    EXPECT_STREQ(retrieved_value, value2);
    free(retrieved_value);
}

TEST_F(SplitCacheTest, UpdateTtl) {
    const char *key = "update_ttl_key";
    const char *value = "update_ttl_value";

    // Put with a TTL, then update with no TTL
    ASSERT_EQ(splitcache_put(cache, key, value, 1), 0);
    ASSERT_EQ(splitcache_put(cache, key, value, 0), 0);
    sleep(2);
    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    free(retrieved_value);

    // Put with no TTL, then update with a TTL
    ASSERT_EQ(splitcache_put(cache, key, value, 0), 0);
    ASSERT_EQ(splitcache_put(cache, key, value, 1), 0);
    sleep(2);
    retrieved_value = splitcache_get(cache, key);
    ASSERT_EQ(retrieved_value, nullptr);
}

TEST_F(SplitCacheTest, DeleteNonExistent) {
    ASSERT_EQ(splitcache_delete(cache, "non_existent_key"), 0);
}

TEST_F(SplitCacheTest, EmptyValue) {
    const char *key = "empty_value_key";
    const char *value = "";

    ASSERT_EQ(splitcache_put(cache, key, value, 0), 0);
    char *retrieved_value = splitcache_get(cache, key);
    ASSERT_NE(retrieved_value, nullptr);
    EXPECT_STREQ(retrieved_value, value);
    free(retrieved_value);
}

} // namespace
