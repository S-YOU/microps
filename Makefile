PROGRAM = echo_server

OBJECTS = microps.o tcp.o udp.o icmp.o ip.o arp.o ethernet.o util.o dhcp.o

CFLAGS  := $(CFLAGS) -O3 -fPIE -W -Wall -Wno-unused-parameter

ifeq ($(shell uname),Linux)
	MAIN_OBJECTS := $(OBJECTS) pkt.o
	CFLAGS  := $(CFLAGS) -lpthread -pthread
endif

ifeq ($(shell uname),Darwin)
	MAIN_OBJECTS := $(OBJECTS) bpf.o
endif


.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all clean

all: $(PROGRAM)

$(PROGRAM): % : %.o $(MAIN_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< $(MAIN_OBJECTS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(PROGRAM) $(PROGRAM:=.o) $(MAIN_OBJECTS)

lib: $(OBJECTS)
	rm libmicrops.a
	$(AR) rs libmicrops.a $(OBJECTS)
