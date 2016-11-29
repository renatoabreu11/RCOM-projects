CC = gcc
CFLAGS  = -g -Wall

default: project

project: main.o urlParser.o
	$(CC) $(CFLAGS) -o main main.o urlParser.o

main.o:  main.c
	$(CC) $(CFLAGS) -c main.c

linkLayer.o: urlParser.h urlParser.c
	$(CC) $(CFLAGS) -c urlParser.c

clean: 
	$(RM) project *.o *~
