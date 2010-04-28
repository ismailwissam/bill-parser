# Makefile for huawei

TARGET = huawei.so
SRCS = $(wildcard ./*.c)
OBJECTS = $(SRCS:%.c=%.o)

CC       = gcc
CFLAGS   = -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline -fPIC

all:$(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -shared -lc -o $(TARGET) $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)

