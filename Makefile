CC = gcc
CXX = g++
CFLAGS = -Iinclude -Ivendor/leveldb/include -Ivendor/googletest/googletest/include -Ivendor/googletest/googletest -Ivendor/uthash/src -Wall -g
CXXFLAGS = $(CFLAGS) -std=c++17 -isystem vendor/leveldb/third_party/benchmark/include
LDFLAGS = -lstdc++ -pthread

SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = build

LIB_NAME = libsplitcache.a
LIB_TARGET = $(LIB_DIR)/$(LIB_NAME)

LEVELDB_LIB = vendor/leveldb/libleveldb.a

SRC_FILES = src/splitcache.c
OBJ_FILES = $(patsubst src/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES))

GTEST_SRC_FILES = vendor/googletest/googletest/src/gtest-all.cc vendor/googletest/googletest/src/gtest_main.cc
GTEST_OBJ_FILES = $(patsubst vendor/googletest/googletest/src/%.cc,$(OBJ_DIR)/vendor/googletest/googletest/src/%.o,$(GTEST_SRC_FILES))

TEST_SRC_FILES = tests/gtest_splitcache.cpp
TEST_OBJ_FILES = $(patsubst tests/%.cpp,$(OBJ_DIR)/tests/%.o,$(TEST_SRC_FILES))
TEST_RUNNER = $(BIN_DIR)/test_runner

BENCHMARK_SRC_FILES = $(filter-out %/benchmark_main.cc, $(wildcard vendor/leveldb/third_party/benchmark/src//*.cc))
BENCHMARK_OBJ_FILES = $(patsubst vendor/leveldb/third_party/benchmark/src/%.cc,$(OBJ_DIR)/vendor/leveldb/third_party/benchmark/src/%.o,$(BENCHMARK_SRC_FILES))

GBENCHMARK_SRC = benchmark/gbenchmark.cpp
GBENCHMARK_OBJ = $(patsubst benchmark/%.cpp,$(OBJ_DIR)/benchmark/%.o,$(GBENCHMARK_SRC))
BENCHMARK_RUNNER = $(BIN_DIR)/benchmark_runner

.PHONY: all clean test leveldb benchmark

all: $(LIB_TARGET)

leveldb:
	@echo "Building leveldb and its dependencies..."
	@cd vendor/leveldb && cmake -DCMAKE_CXX_FLAGS=-fPIC . > /dev/null && $(MAKE) --no-print-directory

$(LIB_TARGET): $(OBJ_FILES)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $(OBJ_FILES)

$(OBJ_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(OBJ_DIR)/benchmark/%.o: benchmark/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/vendor/googletest/googletest/src/%.o: vendor/googletest/googletest/src/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/vendor/leveldb/third_party/benchmark/src/%.o: vendor/leveldb/third_party/benchmark/src/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: leveldb $(LIB_TARGET) $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(LIB_TARGET) $(GTEST_OBJ_FILES) $(TEST_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(GTEST_OBJ_FILES) $(TEST_OBJ_FILES) $(LIB_TARGET) $(LEVELDB_LIB) $(LDFLAGS)

benchmark: leveldb $(LIB_TARGET) $(BENCHMARK_RUNNER)
	./$(BENCHMARK_RUNNER) --benchmark_min_time=2 --benchmark_repetitions=3 --benchmark_display_aggregates_only=true

$(BENCHMARK_RUNNER): $(LIB_TARGET) $(BENCHMARK_OBJ_FILES) $(GBENCHMARK_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(BENCHMARK_OBJ_FILES) $(GBENCHMARK_OBJ) $(LIB_TARGET) $(LEVELDB_LIB) $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "Cleaning leveldb..."
	@$(MAKE) -C vendor/leveldb clean --no-print-directory