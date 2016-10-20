CC = gcc
CFLAGS  = -g -Wall

default: project

project: main.o linkLayer.o applicationLayer.o
	$(CC) $(CFLAGS) -o main main.o linkLayer.o applicationLayer.o

main.o:  main.c
	$(CC) $(CFLAGS) -c main.c

applicationLayer.o: applicationLayer.h applicationLayer.c
	$(CC) $(CFLAGS) -c applicationLayer.c

linkLayer.o: linkLayer.h linkLayer.c
	$(CC) $(CFLAGS) -c linkLayer.c

clean: 
	$(RM) project *.o *~
