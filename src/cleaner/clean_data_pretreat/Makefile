# Makefile for clean_data_pretreat

TARGET = ../../../release/cleaner/clean_data_pretreat
SRCS = $(wildcard ./*.c)
OBJECTS = $(SRCS:%.c=%.o)

CC       = gcc
CFLAGS   += -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline

LIBS     = -B dynamic -lnsl -ldl -lm

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LIBS) $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
