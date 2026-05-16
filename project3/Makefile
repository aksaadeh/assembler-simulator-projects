OBJ = main.o cpu.o shell.o instructions.o

simulator: $(OBJ)
	gcc -o simulator $(OBJ)

main.o: main.c cpu.h shell.h
	gcc -c main.c

cpu.o: cpu.c cpu.h
	gcc -c cpu.c

shell.o: shell.c cpu.h instructions.h
	gcc -c shell.c

instructions.o: instructions.c cpu.h
	gcc -c instructions.c

clean:
	rm -f *.o simulator