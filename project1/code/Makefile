all: assembler

assembler: main.o assembler.o utils.o symbol_table.o
	gcc -o assembler main.o assembler.o utils.o symbol_table.o

main.o: main.c assembler.h
	gcc -c main.c

assembler.o: assembler.c assembler.h utils.h symbol_table.h
	gcc -c assembler.c

utils.o: utils.c utils.h
	gcc -c utils.c

symbol_table.o: symbol_table.c symbol_table.h
	gcc -c symbol_table.c

clean:
	rm -f *.o assembler