CC = gcc

CFLAGS = -Wall

all: applicationLayer linkLayer

applicationLayer: applicationLayer.c 
	$(CC) $(CFLAGS) -o bin/applicationLayer applicationLayer.c 

linkLayer: linkLayer.c
	$(CC) $(CFLAGS) -o bin/linkLayer linkLayer.c

clean: $(RM) bin/* 