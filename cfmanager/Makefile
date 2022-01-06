
CC = $(CROSS)gcc
STRIP = $(CROSS)strip
override CFLAGS +=  -g -O0 -Wall -Werror $(GSFLAGS) -I${PUBLIC_INCLUDE}/usr

LIBS = -lubus -lblobmsg_json -ljson-c -lubox -pthread -luci -lm -lgs_utils -lgs_ipc -lnbrd -lgs_crypto -ldl

OBJS = \
	main.o \
	ubus.o \
	cfparse.o \
	initd.o \
	track.o \
	ipc.o \
	wireless.o \
	utils.o \
	sgrequest.o \
	config.o \
	network.o \
	dhcp.o \
	firewall.o \
	mwan3.o \
	schedule.o \
	qos.o \
	global_access.o \
	cfmanager.o \
	time.o \
	apply.o \
	system.o \
	email.o \
	notification.o \
	grandstream.o \
	controller.o \
	check.o \
	vpn.o \
	upnpd.o \
	route.o \
	bwctrl.o \
	ddns.o \
	usb.o	\
	smb.o  \
	gsportalcfg.o \
	tr069.o \
	ecm.o \
	vpn_server.o \
	cm2gs.o

#OBJS = \
	initd.o \
	track.o \
	utils.o


.SUFFIXES: .c

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: cfmanager

clean:
	rm -f *.o
	rm -f cfmanager

cfmanager: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
