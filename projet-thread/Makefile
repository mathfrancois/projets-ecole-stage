CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -fPIC -Isrc
LDFLAGS=-lthread

SRCS=$(wildcard src/*.c)
SRC_OBJS=$(SRCS:$(src)/%.c=$(build)/%.o)

TESTS=$(TEST_PASS:%=test/%.c) $(TEST_PASS_1PARAM:%=test/%.c) $(TEST_PASS_2PARAM:%=test/%.c)
TESTS_OBJS=$(TESTS:test/%.c=build/test/%.o)
TESTS_EXEC=$(TESTS:test/%.c=build/test/%)
TEST_EXEC_DIR=build/test

TEST_PASS=	01-main		\
			02-switch	\
			03-equity	\
			11-join 	\
			12-join-main	\
			13-join-switch	\
			63-mutex-equity	\
			64-mutex-join
			# 81-deadlock
TEST_PASS_1PARAM= 	21-create-many \
					22-create-many-recursive \
					23-create-many-once	\
					51-fibonacci	\
					71-preemption \
					61-mutex 	\
					62-mutex	
TEST_PASS_2PARAM=	31-switch-many	\
					32-switch-many-join	\
					33-switch-many-cascade

VALGRIND_FLAGS = --leak-check=full --show-reachable=yes --track-origins=yes
PARAM1= 10
PARAM2= 10
LIB_TARGET= build/libthread.a

ENABLE_LOGGING ?= 0
ENABLE_NDEBUG ?= 0
ENABLE_PREEMPTION ?= 1
ifeq ($(ENABLE_PREEMPTION),1)
CFLAGS += -DPREEMPTION_ENABLED
endif
ifeq ($(ENABLE_LOGGING),1)
CFLAGS += -DLOGGING_ENABLED
endif
ifeq ($(ENABLE_NDEBUG),1)
CFLAGS += -DNDEBUG
endif
# make ENABLE_LOGGING=1 ENABLE_NDEBUG=1 ENABLE_PREEMPTION=0

$(shell mkdir -p 	build 	\
					build/test 	\
					build/test_pthread 	\
					build/test_thread_ext/	\
					install/bin 	\
					install/lib)


all: $(LIB_TARGET) test
	
# compile la lib thread
build/libthread.a: build/thread.o
	ar rcs build/libthread.a build/thread.o
build/thread.o: src/thread.c
	$(CC) $(CFLAGS) -c src/thread.c -o build/thread.o

# compile les tests
test: $(TESTS_EXEC)
$(TEST_EXEC_DIR)/%.o: test/%.c src/thread.h
	$(CC) $(CFLAGS) -c $< -o $@ 
$(TEST_EXEC_DIR)/%: $(TEST_EXEC_DIR)/%.o build/libthread.a
	$(CC) $^ -o $@ -Lbuild $(LDFLAGS)


check: $(TESTS_EXEC)
	for exec in $(TEST_PASS); do ./$(TEST_EXEC_DIR)/$$exec; done;
	for exec in $(TEST_PASS_1PARAM); do ./$(TEST_EXEC_DIR)/$$exec $(PARAM1); done;
	for exec in $(TEST_PASS_2PARAM); do ./$(TEST_EXEC_DIR)/$$exec $(PARAM1) $(PARAM2); done;


valgrind: test
	for exec in $(TEST_PASS); do valgrind $(VALGRIND_FLAGS) ./$(TEST_EXEC_DIR)/$$exec; done;
	for exec in $(TEST_PASS_1PARAM); do valgrind $(VALGRIND_FLAGS) ./$(TEST_EXEC_DIR)/$$exec $(PARAM1); done;
	for exec in $(TEST_PASS_2PARAM); do valgrind $(VALGRIND_FLAGS) ./$(TEST_EXEC_DIR)/$$exec $(PARAM2) $(PARAM2); done;


# compile les tests avec pthread
TESTS_EXEC_PTHREAD=$(TESTS:test/%.c=build/test_pthread/%)
pthread: $(TESTS_EXEC_PTHREAD)
$(TEST_EXEC_DIR)_pthread/%.o: test/%.c src/thread.h
	$(CC) $(CFLAGS) -c $< -o $@ -lpthread -DUSE_PTHREAD
$(TEST_EXEC_DIR)_pthread/%: $(TEST_EXEC_DIR)_pthread/%.o build/libthread.a
	$(CC) $^ -o $@ -Lbuild 

# lance les benchmarks sur tous les tests. 
graphs: 
	$(echo "Benchmark pour tous les tests lancés ")
	./benchmark/launch_benchmark.sh all 0.5 

# lance les benchmarks et trace les graphes sur executable 
graphs_%: 
	$(echo "Benchmark pour $(patsubst graphs_%,%,$@) lancés ")
	./benchmark/launch_benchmark.sh $(patsubst graphs_%,%,$@) 0.5 


install: all
	cp build/libthread.a install/lib/
	cp build/test/* install/bin/



## same for thread_ext
thread_ext: build/libthread_ext.a test_ext
# compile la lib thread_ext
build/libthread_ext.a: build/thread_ext.o
	ar rcs build/libthread_ext.a build/thread_ext.o
build/thread_ext.o: src/thread_ext.c
	$(CC) $(CFLAGS) -c src/thread_ext.c -o build/thread_ext.o
# compile les tests ext
test_ext: $(TESTS_EXEC_EXT)
TEST_EXEC_DIR_EXT=build/test_thread_ext
TEST_EXT= $(TEST_PASS_EXT:%=test/%.c) $(TEST_PASS1_EXT:%=test/%.c)
TEST_PASS_EXT= 	priority-test	\
				priority_test_basic \
				priority_test_starvation \
				priority_test_stress
TEST_PASS1_EXT= priority_test_efficacity
TESTS_OBJS_EXT=$(TEST_EXT:test/%.c=build/test_thread_ext/%.o)
TESTS_EXEC_EXT=$(TEST_EXT:test/%.c=build/test_thread_ext/%)
test_ext: $(TESTS_EXEC_EXT)
$(TEST_EXEC_DIR_EXT)/%.o: test/%.c src/thread_ext.h
	$(CC) $(CFLAGS) -c $< -o $@ 
$(TEST_EXEC_DIR_EXT)/%: $(TEST_EXEC_DIR_EXT)/%.o build/libthread_ext.a
	$(CC) $^ -o $@ -Lbuild -lthread_ext
check_ext: $(TESTS_EXEC_EXT)
	for exec in $(TEST_PASS_EXT); do ./$(TEST_EXEC_DIR_EXT)/$$exec; done;
	for exec in $(TEST_PASS1_EXT); do ./$(TEST_EXEC_DIR_EXT)/$$exec $(PARAM1); done;
valgrind_ext: test_ext
	for exec in $(TEST_PASS_EXT); do valgrind $(VALGRIND_FLAGS) ./$(TEST_EXEC_DIR_EXT)/$$exec; done;
	for exec in $(TEST_PASS1_EXT); do valgrind $(VALGRIND_FLAGS) ./$(TEST_EXEC_DIR_EXT)/$$exec $(PARAM1); done;




clean:
	rm -rf build/


.PHONY: all check valgrind pthreads graphs install clean 
