CC=gcc 
CFLAGS += -D${flag}
all : main

%.o: %.c
	$(CC) -c $< -o $@ ${CFLAGS}
main: test.o
	$(CC) $^ -o $@  ${CFLAGS}

clean:
	rm -f *.o

