CC=gcc
SRC=$(wildcard *.c)
OBJ=$(patsubst %c,%o,$(SRC))
main:$(OBJ)
	$(CC) -o $@ -I./include $(OBJ)

%.o:%.c 
	$(CC) -c $< -I./include 

libcal.a:sub.o add.o
	ar -r libcal.a add.o sub.o

libcal.so:sub.o add.o 
	gcc -shared add.o sub.o -o libcal.so

test:
	@echo $(SRC)
	@echo $(OBJ)

clean:
	rm -f *.o
	rm -f *.a 
	rm -f *.so