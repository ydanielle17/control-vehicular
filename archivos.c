#include <stdio.h>
#include "archivos.h"

// regresa cuantos propietarios se leyeron
int leer_propietarios(Propietario lista[])
{
    FILE *f = fopen(ARCHIVO_PROPIETARIOS, "rb");
    if (f == NULL)
        return 0;   // todavia no hay archivo
    int n = fread(lista, sizeof(Propietario), MAX_REGISTROS, f);
    fclose(f);
    return n;
}

int guardar_propietario(Propietario *p)
{
    FILE *f = fopen(ARCHIVO_PROPIETARIOS, "ab");
    if (f == NULL)
        return -1;
    fwrite(p, sizeof(Propietario), 1, f);
    fclose(f);
    return 0;
}

int leer_vehiculos(Vehiculo lista[])
{
    FILE *f = fopen(ARCHIVO_VEHICULOS, "rb");
    if (f == NULL)
        return 0;
    int n = fread(lista, sizeof(Vehiculo), MAX_REGISTROS, f);
    fclose(f);
    return n;
}

int guardar_vehiculo(Vehiculo *v)
{
    FILE *f = fopen(ARCHIVO_VEHICULOS, "ab");
    if (f == NULL)
        return -1;
    fwrite(v, sizeof(Vehiculo), 1, f);
    fclose(f);
    return 0;
}
