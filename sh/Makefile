CC = gcc 
LIBS = -L./lib -lblobmsg_json -lubox -ljson_script -lubus 
all : ubus
ubus.o : ubus.c 
	$(CC)  -c ubus.c
ubus : ubus.o
	$(CC) ubus.o -o ubus $(LIBS)
