#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include "comun.h"

#define ARCHIVO_PROPIETARIOS "propietarios.dat"
#define ARCHIVO_VEHICULOS    "vehiculos.dat"

int leer_propietarios(Propietario lista[]);
int guardar_propietario(Propietario *p);
int leer_vehiculos(Vehiculo lista[]);
int guardar_vehiculo(Vehiculo *v);

#endif
