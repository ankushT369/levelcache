CC=gcc
CFLAGS=-Iinclude -Ivendor/leveldb/include -Wall -g
LDFLAGS=-Lvendor/leveldb/build -lleveldb -lstdc++

SRC_DIR=src
LIB_DIR=lib
BIN_DIR=bin
OBJ_DIR=build

LIB_NAME=libsplitcache.so
LIB_TARGET=$(LIB_DIR)/$(LIB_NAME)
LEVELDB_LIB=vendor/leveldb/build/libleveldb.a

SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TEST_SRC_FILES=$(wildcard tests/*.c)
TEST_RUNNER=$(BIN_DIR)/test_runner

.PHONY: all clean test vendor

all: $(LIB_TARGET)

vendor: $(LEVELDB_LIB)

$(LEVELDB_LIB):
	cd vendor/leveldb && mkdir -p build && cd build && cmake .. && cmake --build .

# Build the shared library
$(LIB_TARGET): $(OBJ_FILES) $(LEVELDB_LIB)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $(OBJ_FILES) $(LDFLAGS)

# Compile source files with -fPIC for the shared library
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Build and run tests
test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

$(TEST_RUNNER): $(SRC_FILES) $(TEST_SRC_FILES) $(LEVELDB_LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	rm -rf vendor/leveldb/build
