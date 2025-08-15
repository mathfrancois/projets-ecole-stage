TARGET = kinematics

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

SRC = kinematics.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
