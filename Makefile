CC = gcc
CXX = g++
CFLAGS = -Iinclude -Ivendor/leveldb/include -Ivendor/googletest/googletest/include -Ivendor/googletest/googletest -Ivendor/uthash/src -Wall -g -std=c++17
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

.PHONY: all clean test leveldb

all: $(LIB_TARGET)

leveldb:
	@echo "Building leveldb..."
	@cd vendor/leveldb && cmake -DCMAKE_CXX_FLAGS=-fPIC . > /dev/null && $(MAKE) --no-print-directory

$(LIB_TARGET): $(OBJ_FILES)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $(OBJ_FILES)

$(OBJ_DIR)/src/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/vendor/googletest/googletest/src/%.o: vendor/googletest/googletest/src/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) -c $< -o $@

test: leveldb $(LIB_TARGET) $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(LIB_TARGET) $(GTEST_OBJ_FILES) $(TEST_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $(GTEST_OBJ_FILES) $(TEST_OBJ_FILES) $(LIB_TARGET) $(LEVELDB_LIB) $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "Cleaning leveldb..."
	@$(MAKE) -C vendor/leveldb clean --no-print-directory
