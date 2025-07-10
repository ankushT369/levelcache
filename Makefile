CC = gcc
CXX = g++
CFLAGS = -Iinclude -Ivendor/leveldb/include -Ivendor/googletest/googletest/include -Ivendor/uthash/src -Wall -g
LDFLAGS = -lstdc++ -pthread

SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = build

LIB_NAME = libsplitcache.a
LIB_TARGET = $(LIB_DIR)/$(LIB_NAME)

LEVELDB_LIB = vendor/leveldb/libleveldb.a
GTEST_LIB = vendor/leveldb/lib/libgtest.a
GTEST_MAIN_LIB = vendor/leveldb/lib/libgtest_main.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TEST_SRC_FILES = $(wildcard tests/*.c)
TEST_RUNNER = $(BIN_DIR)/test_runner

.PHONY: all clean test leveldb

all: $(LIB_TARGET)

leveldb:
	cd vendor/leveldb && cmake -DCMAKE_CXX_FLAGS=-fPIC . && make

$(LIB_TARGET): $(OBJ_FILES)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

test: leveldb $(LIB_TARGET) $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(TEST_SRC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIB_TARGET) $(LEVELDB_LIB) $(GTEST_LIB) $(GTEST_MAIN_LIB) $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	$(MAKE) -C vendor/leveldb clean