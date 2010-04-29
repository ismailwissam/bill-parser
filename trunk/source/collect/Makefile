# Makefile for collect

TARGET = ../../release/collect/collect
SRCS = $(wildcard ./*.c)
OBJECTS = $(SRCS:%.c=%.o)

CC       = gcc
CFLAGS   = -O2 -Wall -W -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline 
LDFLAGS  = -L.
# LIBS     = -lFtp -lresolv -lsocket -lnsl
LIBS     = -lFtp -lresolv -lnsl

all: $(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(LIBS) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS)

