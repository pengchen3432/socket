CC = $(CROSS)gcc

CFLAGS =    -g -O0 -Wall -I$(PUBLIC_INCLUDE)/usr
LIBS = -lm -luci -lubus -lblobmsg_json -ljson-c -lubox -lgs_utils
all: mdns
	
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

mdns: main.o mdns.o config_init.o forward_message.o 
	$(CC) $(CFLAGS) $^ -o $@    $(LIBS)

clean:
	rm -f *.o
