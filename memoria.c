#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "memoria.h"

Memoria *memoria = NULL;
sem_t *turno = NULL;
sem_t *peticion = NULL;
sem_t *respuesta = NULL;

int crear_memoria(void)
{
    // por si quedaron de una ejecucion anterior
    shm_unlink(NOMBRE_MEMORIA);
    sem_unlink(SEM_TURNO);
    sem_unlink(SEM_PETICION);
    sem_unlink(SEM_RESPUESTA);

    int fd = shm_open(NOMBRE_MEMORIA, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return -1;
    }
    if (ftruncate(fd, sizeof(Memoria)) == -1) {
        perror("ftruncate");
        close(fd);
        return -1;
    }
    memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (memoria == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    memset(memoria, 0, sizeof(Memoria));

    turno = sem_open(SEM_TURNO, O_CREAT, 0666, 1);
    peticion = sem_open(SEM_PETICION, O_CREAT, 0666, 0);
    respuesta = sem_open(SEM_RESPUESTA, O_CREAT, 0666, 0);
    if (turno == SEM_FAILED || peticion == SEM_FAILED || respuesta == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }
    return 0;
}

void borrar_memoria(void)
{
    munmap(memoria, sizeof(Memoria));
    sem_close(turno);
    sem_close(peticion);
    sem_close(respuesta);
    shm_unlink(NOMBRE_MEMORIA);
    sem_unlink(SEM_TURNO);
    sem_unlink(SEM_PETICION);
    sem_unlink(SEM_RESPUESTA);
}

int abrir_memoria(void)
{
    int fd = shm_open(NOMBRE_MEMORIA, O_RDWR, 0666);
    if (fd == -1)
        return -1;
    memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (memoria == MAP_FAILED)
        return -1;

    turno = sem_open(SEM_TURNO, 0);
    peticion = sem_open(SEM_PETICION, 0);
    respuesta = sem_open(SEM_RESPUESTA, 0);
    if (turno == SEM_FAILED || peticion == SEM_FAILED || respuesta == SEM_FAILED)
        return -1;
    return 0;
}

void cerrar_memoria(void)
{
    munmap(memoria, sizeof(Memoria));
    sem_close(turno);
    sem_close(peticion);
    sem_close(respuesta);
}
