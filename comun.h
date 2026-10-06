#ifndef COMUN_H
#define COMUN_H

#define MAX_REGISTROS 200

#define TAM_IDENT     21
#define TAM_NOMBRE    61
#define TAM_TELEFONO  11
#define TAM_CORREO    81
#define TAM_ESCUELA   61
#define TAM_PLACA     11
#define TAM_MARCA     31
#define TAM_MODELO    31
#define TAM_COLOR     21

#define NUM_TIPOS_PROPIETARIO 7
#define NUM_TIPOS_VEHICULO    8

typedef struct {
    int id;
    int tipo;
    char identificador[TAM_IDENT];   // boleta, numero de empleado o INE
    char nombre[TAM_NOMBRE];
    char apellidos[TAM_NOMBRE];
    char telefono[TAM_TELEFONO];
    char correo[TAM_CORREO];
    char escuela[TAM_ESCUELA];
} Propietario;

typedef struct {
    int id;
    int tipo;
    char placa[TAM_PLACA];
    char marca[TAM_MARCA];
    char modelo[TAM_MODELO];
    int anio;
    char color[TAM_COLOR];
    int id_propietario;
} Vehiculo;

extern const char *tipos_propietario[NUM_TIPOS_PROPIETARIO];
extern const char *tipos_vehiculo[NUM_TIPOS_VEHICULO];

int es_numero(const char *texto);
int solo_letras(const char *texto);
int identificador_valido(const char *texto);
int telefono_valido(const char *texto);
int correo_valido(const char *texto);
int placa_valida(const char *texto);
int requiere_placa(int tipo);

#endif
