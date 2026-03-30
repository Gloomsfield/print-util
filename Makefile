CC = clang
CFLAGS = -Wall -std=c99 -lusb-1.0 -lsystemd
SRC_FILES = src/main.c src/pu.c
OBJ_FILES = build/main.o build/pu.o

TARGET = build/pu

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(TARGET)

-include $(OBJS:%.o=%.d)

build/%.o: src/%.c
	$(CC) -MMD -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ_FILES)


