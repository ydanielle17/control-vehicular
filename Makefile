CC = gcc
CFLAGS = -Wall -std=c11 -D_GNU_SOURCE

all: servidor cliente

servidor: servidor.c archivos.c memoria.c comun.c archivos.h memoria.h comun.h
	$(CC) $(CFLAGS) -o servidor servidor.c archivos.c memoria.c comun.c -pthread -lrt

cliente: cliente.c memoria.c comun.c memoria.h comun.h
	$(CC) $(CFLAGS) -o cliente cliente.c memoria.c comun.c -pthread -lrt -lncursesw

clean:
	rm -f servidor cliente
