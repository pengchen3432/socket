CC=gcc
SRC=$(wildcard *.c)
OBJ=$(patsubst %c,%o,$(SRC))
main:$(OBJ)
	$(CC) -o $@ -I./include $(OBJ)

%.o:%.c 
	$(CC) -c $< -I./include 

test:
	@echo $(SRC)
	@echo $(OBJ)

clean:
	rm -f *.o