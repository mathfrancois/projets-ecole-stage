CC = gcc
CFLAGS = -Wall -Wextra -g -I src -DLOG_USE_COLOR

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests

SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)

$(shell mkdir -p 	$(BUILD_DIR)/src 	\
					$(BUILD_DIR)/tests	)
$(shell touch serveur.log)

all: serveur

# Compilation du serveur
serveur: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# Compilation et exécution des tests
test: $(BUILD_DIR)/test
	./$(BUILD_DIR)/test

$(BUILD_DIR)/test: $(TEST_OBJS) $(filter-out $(BUILD_DIR)/$(SRC_DIR)/serveur.o, $(OBJS))
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./serveur

clean:
	rm -rf $(BUILD_DIR) serveur serveur.log

.PHONY: all test clean
