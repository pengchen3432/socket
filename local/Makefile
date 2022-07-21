
CC = $(CROSS)gcc
STRIP = $(CROSS)strip
override CFLAGS +=  -g -O0 -Wall -Werror $(GSFLAGS) -I${PUBLIC_INCLUDE}/usr


LIBS = -lubus -lblobmsg_json -ljson-c -lubox -pthread -luci -lm -lgs_utils -lgs_ipc -lnbrd -lgs_crypto -ldl

OBJS = \
	main.o


#OBJS = \
	initd.o \
	track.o \
	utils.o


.SUFFIXES: .c

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: local

clean:
	rm -f *.o
	rm -f local

local: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
