all: zing zing2
zing2: main.o zing2.o
	gcc -o zing2 zing2.o main.o

zing: main.o zing.o 
	gcc -v zing.o main.o

main.o: main.c
	gcc -v -Wall -c main.c

zing2.o: zing2.c
	gcc -v -Wall -c zing2.c


