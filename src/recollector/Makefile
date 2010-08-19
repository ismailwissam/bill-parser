# Makefile for recollect

TARGET = ../../release/recollector/recollector
SRCS = $(wildcard ./*.c) 
OBJECTS = $(SRCS:%.c=%.o)

CC = gcc
CFLAGS += -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline 
LDFLAGS = -L./
LIBS = -ldl -lm -lz -lFtp -lresolv -lsocket -lnsl

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)

