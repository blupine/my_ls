CC=gcc -std=c99
OBJ=ls.o
Target=ls

$(Target): $(OBJ)
	$(CC) -o $@ $(OBJ)

clean:
	rm -f ./ls ./main.o ./ls.c~ ./Makefile~ ./ls.o

