# Makefile for pretreat

TARGET = pretreat
SRCS = $(wildcard ./*.c) 
OBJECTS = $(SRCS:%.c=%.o)

CC = gcc
CFLAGS += -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline 
LIBS = -ldl

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LIBS) $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)

