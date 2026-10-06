#ifndef MEMORIA_H
#define MEMORIA_H

#include <semaphore.h>
#include "comun.h"

#define NOMBRE_MEMORIA "/control_vehicular"
#define SEM_TURNO      "/cv_turno"
#define SEM_PETICION   "/cv_peticion"
#define SEM_RESPUESTA  "/cv_respuesta"

// operaciones que el cliente le pide al servidor
#define ALTA_PROPIETARIO    1
#define LISTA_PROPIETARIOS  2
#define BUSCAR_PROPIETARIO  3
#define ALTA_VEHICULO       4
#define LISTA_VEHICULOS     5

// lo que se comparte entre el cliente y el servidor
typedef struct {
    int operacion;
    Propietario propietario;   // datos que manda el cliente
    Vehiculo vehiculo;

    int correcto;              // respuesta del servidor
    char mensaje[200];
    int total;
    Propietario propietarios[MAX_REGISTROS];
    Vehiculo vehiculos[MAX_REGISTROS];
} Memoria;

extern Memoria *memoria;
extern sem_t *turno;       // solo un cliente a la vez usa la memoria
extern sem_t *peticion;    // el cliente avisa que dejo una peticion
extern sem_t *respuesta;   // el servidor avisa que ya respondio

int crear_memoria(void);    // servidor
void borrar_memoria(void);  // servidor
int abrir_memoria(void);    // cliente
void cerrar_memoria(void);  // cliente

#endif
