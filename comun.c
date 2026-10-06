#include <ctype.h>
#include <string.h>
#include "comun.h"

const char *tipos_propietario[NUM_TIPOS_PROPIETARIO] = {
    "ALUMNO", "DOCENTE", "PAAE", "DIRECTIVO", "EGRESADO", "VISITANTE", "PROVEEDOR"
};

const char *tipos_vehiculo[NUM_TIPOS_VEHICULO] = {
    "AUTOMOVIL", "CAMIONETA", "MOTOCICLETA", "MOTONETA", "CAMION",
    "BICICLETA", "SCOOTER", "OTRO"
};

int es_numero(const char *texto)
{
    if (strlen(texto) == 0)
        return 0;
    for (int i = 0; texto[i] != '\0'; i++) {
        if (!isdigit((unsigned char)texto[i]))
            return 0;
    }
    return 1;
}

// letras, espacios y acentos (los acentos ocupan bytes mayores a 127)
int solo_letras(const char *texto)
{
    if (strlen(texto) < 2)
        return 0;
    for (int i = 0; texto[i] != '\0'; i++) {
        unsigned char c = texto[i];
        if (!isalpha(c) && c != ' ' && c < 128)
            return 0;
    }
    return 1;
}

int identificador_valido(const char *texto)
{
    if (strlen(texto) < 3)
        return 0;
    for (int i = 0; texto[i] != '\0'; i++) {
        if (!isalnum((unsigned char)texto[i]))
            return 0;
    }
    return 1;
}

int telefono_valido(const char *texto)
{
    return strlen(texto) == 10 && es_numero(texto);
}

int correo_valido(const char *texto)
{
    char *arroba = strchr(texto, '@');
    if (arroba == NULL || arroba == texto)
        return 0;
    if (strchr(arroba, '.') == NULL || strchr(texto, ' ') != NULL)
        return 0;
    return texto[strlen(texto) - 1] != '.';
}

int placa_valida(const char *texto)
{
    if (strlen(texto) < 5)
        return 0;
    for (int i = 0; texto[i] != '\0'; i++) {
        if (!isalnum((unsigned char)texto[i]) && texto[i] != '-')
            return 0;
    }
    return 1;
}

// bicicleta, scooter y otro no llevan placa
int requiere_placa(int tipo)
{
    return tipo < 5;
}
