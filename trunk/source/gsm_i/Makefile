# Makefile for insert

TARGET = ../../release/insert
SRCS = $(wildcard ./*.c)
OBJECTS = $(SRCS:%.c=%.o)

CC       = gcc
CFLAGS   += -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline \
           -I/export/home/sybase/OCS-12_5/include

LDFLAGS  = -L/export/home/sybase/OCS-12_5/lib
LIBS     = -lct -lcs -ltcl -lcomn -lintl -B dynamic -lsocket -lnsl -ldl -lm

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(LIBS) $(OBJECTS)

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
