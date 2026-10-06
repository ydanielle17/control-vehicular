#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "archivos.h"
#include "memoria.h"

volatile sig_atomic_t corriendo = 1;

// Ctrl+C: se avisa con el semaforo para que el servidor deje de esperar
void detener(int senal)
{
    corriendo = 0;
    sem_post(peticion);
}

void responder(int correcto, const char *texto)
{
    memoria->correcto = correcto;
    strcpy(memoria->mensaje, texto);
}

void mayusculas(char *texto)
{
    for (int i = 0; texto[i] != '\0'; i++)
        texto[i] = toupper((unsigned char)texto[i]);
}

void alta_propietario(void)
{
    Propietario p = memoria->propietario;
    Propietario lista[MAX_REGISTROS];
    char texto[200];

    // se vuelven a validar los datos por si acaso
    if (p.tipo < 0 || p.tipo >= NUM_TIPOS_PROPIETARIO) {
        responder(0, "Tipo de propietario no valido");
        return;
    }
    if (!identificador_valido(p.identificador) || !solo_letras(p.nombre) || !solo_letras(p.apellidos)) {
        responder(0, "Datos del propietario no validos");
        return;
    }
    if ((strlen(p.telefono) > 0 && !telefono_valido(p.telefono)) ||
        (strlen(p.correo) > 0 && !correo_valido(p.correo))) {
        responder(0, "Telefono o correo no validos");
        return;
    }

    mayusculas(p.identificador);
    int n = leer_propietarios(lista);
    for (int i = 0; i < n; i++) {
        if (strcmp(lista[i].identificador, p.identificador) == 0) {
            sprintf(texto, "Ya existe un propietario con el identificador %s", p.identificador);
            responder(0, texto);
            return;
        }
    }
    if (n == MAX_REGISTROS) {
        responder(0, "Ya no hay espacio para mas propietarios");
        return;
    }

    p.id = n + 1;
    if (guardar_propietario(&p) == -1) {
        responder(0, "No se pudo guardar en el archivo");
        return;
    }
    sprintf(texto, "Propietario registrado con ID %d", p.id);
    responder(1, texto);
}

void lista_propietarios(void)
{
    memoria->total = leer_propietarios(memoria->propietarios);
    responder(1, "");
}

// busca un propietario por su boleta / numero de empleado
void buscar_propietario(void)
{
    Propietario lista[MAX_REGISTROS];
    int n = leer_propietarios(lista);

    for (int i = 0; i < n; i++) {
        if (strcasecmp(lista[i].identificador, memoria->propietario.identificador) == 0) {
            memoria->propietarios[0] = lista[i];
            responder(1, "Propietario encontrado");
            return;
        }
    }
    responder(0, "No existe un propietario con ese identificador");
}

void alta_vehiculo(void)
{
    Vehiculo v = memoria->vehiculo;
    Vehiculo lista[MAX_REGISTROS];
    Propietario propietarios[MAX_REGISTROS];
    char texto[200];

    if (v.tipo < 0 || v.tipo >= NUM_TIPOS_VEHICULO) {
        responder(0, "Tipo de vehiculo no valido");
        return;
    }
    int num_propietarios = leer_propietarios(propietarios);
    if (v.id_propietario < 1 || v.id_propietario > num_propietarios) {
        responder(0, "El propietario no existe");
        return;
    }
    if (strlen(v.marca) == 0) {
        responder(0, "La marca es obligatoria");
        return;
    }
    if (v.anio != 0 && (v.anio < 1900 || v.anio > 2100)) {
        responder(0, "Anio no valido");
        return;
    }

    int n = leer_vehiculos(lista);
    if (n == MAX_REGISTROS) {
        responder(0, "Ya no hay espacio para mas vehiculos");
        return;
    }
    v.id = n + 1;

    if (strlen(v.placa) == 0) {
        if (requiere_placa(v.tipo)) {
            responder(0, "Este tipo de vehiculo necesita placa");
            return;
        }
        sprintf(v.placa, "IPN-%04d", v.id);   // folio para bicicletas y scooters
    } else if (!placa_valida(v.placa)) {
        responder(0, "Placa no valida");
        return;
    }

    mayusculas(v.placa);
    for (int i = 0; i < n; i++) {
        if (strcmp(lista[i].placa, v.placa) == 0) {
            sprintf(texto, "La placa %s ya esta registrada", v.placa);
            responder(0, texto);
            return;
        }
    }

    if (guardar_vehiculo(&v) == -1) {
        responder(0, "No se pudo guardar en el archivo");
        return;
    }
    sprintf(texto, "Vehiculo registrado con placa %s", v.placa);
    responder(1, texto);
}

void lista_vehiculos(void)
{
    memoria->total = leer_vehiculos(memoria->vehiculos);
    // tambien se mandan los propietarios para mostrar el nombre del dueño
    leer_propietarios(memoria->propietarios);
    responder(1, "");
}

// hilo que atiende la peticion que esta en la memoria compartida
void *atender(void *arg)
{
    switch (memoria->operacion) {
    case ALTA_PROPIETARIO:   alta_propietario();   break;
    case LISTA_PROPIETARIOS: lista_propietarios(); break;
    case BUSCAR_PROPIETARIO: buscar_propietario(); break;
    case ALTA_VEHICULO:      alta_vehiculo();      break;
    case LISTA_VEHICULOS:    lista_vehiculos();    break;
    default:                 responder(0, "Operacion desconocida");
    }
    printf("Peticion %d atendida: %s\n", memoria->operacion, memoria->mensaje);
    return NULL;
}

int main(void)
{
    if (crear_memoria() == -1) {
        printf("No se pudo crear la memoria compartida\n");
        return 1;
    }
    signal(SIGINT, detener);
    printf("Servidor iniciado. Presiona Ctrl+C para terminar.\n");

    while (1) {
        sem_wait(peticion);
        if (!corriendo)
            break;

        pthread_t hilo;
        pthread_create(&hilo, NULL, atender, NULL);
        pthread_join(hilo, NULL);

        sem_post(respuesta);
    }

    borrar_memoria();
    printf("\nServidor terminado.\n");
    return 0;
}
