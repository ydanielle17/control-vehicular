#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoria.h"

// ---------- funciones de pantalla ----------

void titulo(const char *texto)
{
    clear();
    attron(COLOR_PAIR(1) | A_BOLD);
    mvhline(0, 0, ' ', COLS);
    mvprintw(0, 2, "SISTEMA DE CONTROL VEHICULAR - IPN");
    attroff(COLOR_PAIR(1) | A_BOLD);
    attron(A_BOLD);
    mvprintw(2, 2, "%s", texto);
    attroff(A_BOLD);
}

void mostrar_error(const char *texto)
{
    move(LINES - 3, 0);
    clrtoeol();
    attron(COLOR_PAIR(3));
    mvprintw(LINES - 3, 2, "%s", texto);
    attroff(COLOR_PAIR(3));
}

void mostrar_resultado(int correcto, const char *texto)
{
    move(LINES - 3, 0);
    clrtoeol();
    attron(COLOR_PAIR(correcto ? 2 : 3) | A_BOLD);
    mvprintw(LINES - 3, 2, "%s", texto);
    attroff(COLOR_PAIR(correcto ? 2 : 3) | A_BOLD);
    mvprintw(LINES - 2, 2, "Presiona una tecla para continuar...");
    getch();
}

void leer_texto(int fila, const char *etiqueta, char *destino, int max)
{
    move(fila, 0);
    clrtoeol();
    mvprintw(fila, 2, "%s: ", etiqueta);
    echo();
    curs_set(1);
    getnstr(destino, max);
    noecho();
    curs_set(0);
    move(LINES - 3, 0);   // borra el error anterior
    clrtoeol();
}

int elegir_opcion(int fila, const char *etiqueta, const char *opciones[], int n)
{
    mvprintw(fila, 2, "%s:", etiqueta);
    for (int i = 0; i < n; i++)
        mvprintw(fila + 1 + i, 4, "%d) %s", i + 1, opciones[i]);
    mvprintw(fila + n + 1, 2, "Elige una opcion: ");

    int tecla;
    do {
        tecla = getch();
    } while (tecla < '1' || tecla >= '1' + n);

    // se borran las opciones y se deja solo la elegida
    for (int i = 0; i <= n + 1; i++) {
        move(fila + i, 0);
        clrtoeol();
    }
    mvprintw(fila, 2, "%s: %s", etiqueta, opciones[tecla - '1']);
    return tecla - '1';
}

// manda la peticion al servidor y espera la respuesta
void enviar_peticion(void)
{
    sem_post(peticion);
    sem_wait(respuesta);
}

// ---------- propietarios ----------

void registrar_propietario(void)
{
    Propietario p;
    memset(&p, 0, sizeof(p));
    titulo("Registrar propietario");

    p.tipo = elegir_opcion(4, "Tipo de propietario", tipos_propietario, NUM_TIPOS_PROPIETARIO);

    do {
        leer_texto(5, "Boleta / No. de empleado / INE", p.identificador, TAM_IDENT - 1);
        if (!identificador_valido(p.identificador))
            mostrar_error("Solo letras y numeros, minimo 3 caracteres");
    } while (!identificador_valido(p.identificador));

    do {
        leer_texto(6, "Nombre(s)", p.nombre, TAM_NOMBRE - 1);
        if (!solo_letras(p.nombre))
            mostrar_error("El nombre solo puede tener letras");
    } while (!solo_letras(p.nombre));

    do {
        leer_texto(7, "Apellidos", p.apellidos, TAM_NOMBRE - 1);
        if (!solo_letras(p.apellidos))
            mostrar_error("Los apellidos solo pueden tener letras");
    } while (!solo_letras(p.apellidos));

    do {
        leer_texto(8, "Telefono (10 digitos, opcional)", p.telefono, TAM_TELEFONO - 1);
        if (strlen(p.telefono) > 0 && !telefono_valido(p.telefono))
            mostrar_error("El telefono debe tener 10 digitos");
    } while (strlen(p.telefono) > 0 && !telefono_valido(p.telefono));

    do {
        leer_texto(9, "Correo (opcional)", p.correo, TAM_CORREO - 1);
        if (strlen(p.correo) > 0 && !correo_valido(p.correo))
            mostrar_error("Correo no valido, ejemplo: nombre@alumno.ipn.mx");
    } while (strlen(p.correo) > 0 && !correo_valido(p.correo));

    leer_texto(10, "Escuela o area", p.escuela, TAM_ESCUELA - 1);

    sem_wait(turno);
    memoria->operacion = ALTA_PROPIETARIO;
    memoria->propietario = p;
    enviar_peticion();
    int correcto = memoria->correcto;
    char mensaje[200];
    strcpy(mensaje, memoria->mensaje);
    sem_post(turno);

    mostrar_resultado(correcto, mensaje);
}

void ver_propietarios(void)
{
    Propietario lista[MAX_REGISTROS];
    int total;

    sem_wait(turno);
    memoria->operacion = LISTA_PROPIETARIOS;
    enviar_peticion();
    total = memoria->total;
    memcpy(lista, memoria->propietarios, total * sizeof(Propietario));
    sem_post(turno);

    titulo("Lista de propietarios");
    attron(A_UNDERLINE);
    mvprintw(4, 2, "%-4s %-11s %-14s %-35s %-11s", "ID", "TIPO", "IDENTIFICADOR", "NOMBRE", "TELEFONO");
    attroff(A_UNDERLINE);

    if (total == 0)
        mvprintw(6, 2, "No hay propietarios registrados");
    for (int i = 0; i < total && 5 + i < LINES - 4; i++) {
        char nombre[2 * TAM_NOMBRE];
        sprintf(nombre, "%s %s", lista[i].nombre, lista[i].apellidos);
        mvprintw(5 + i, 2, "%-4d %-11s %-14s %-35s %-11s", lista[i].id, tipos_propietario[lista[i].tipo],
                 lista[i].identificador, nombre, lista[i].telefono);
    }
    mvprintw(LINES - 2, 2, "Total: %d. Presiona una tecla para regresar...", total);
    getch();
}

// ---------- vehiculos ----------

void registrar_vehiculo(void)
{
    Vehiculo v;
    char identificador[TAM_IDENT];
    char anio[5];
    char mensaje[200];
    int correcto;
    memset(&v, 0, sizeof(v));
    titulo("Registrar vehiculo");

    // primero se busca al dueño
    leer_texto(4, "Boleta / No. de empleado / INE del propietario", identificador, TAM_IDENT - 1);
    sem_wait(turno);
    memoria->operacion = BUSCAR_PROPIETARIO;
    strcpy(memoria->propietario.identificador, identificador);
    enviar_peticion();
    correcto = memoria->correcto;
    Propietario dueno = memoria->propietarios[0];
    sem_post(turno);

    if (!correcto) {
        mostrar_resultado(0, "No existe ese propietario, registralo primero");
        return;
    }
    v.id_propietario = dueno.id;
    mvprintw(4, 2, "Propietario: %s %s", dueno.nombre, dueno.apellidos);
    clrtoeol();

    v.tipo = elegir_opcion(6, "Tipo de vehiculo", tipos_vehiculo, NUM_TIPOS_VEHICULO);

    do {
        if (requiere_placa(v.tipo))
            leer_texto(7, "Placa", v.placa, TAM_PLACA - 1);
        else
            leer_texto(7, "Placa (Enter si no tiene)", v.placa, TAM_PLACA - 1);

        if (strlen(v.placa) == 0 && requiere_placa(v.tipo))
            mostrar_error("La placa es obligatoria para este tipo de vehiculo");
        else if (strlen(v.placa) > 0 && !placa_valida(v.placa))
            mostrar_error("Placa no valida: solo letras, numeros y guiones, minimo 5");
        else
            break;
    } while (1);

    do {
        leer_texto(8, "Marca", v.marca, TAM_MARCA - 1);
        if (strlen(v.marca) == 0)
            mostrar_error("La marca es obligatoria");
    } while (strlen(v.marca) == 0);

    leer_texto(9, "Modelo", v.modelo, TAM_MODELO - 1);

    do {
        leer_texto(10, "Anio (Enter si no aplica)", anio, 4);
        v.anio = atoi(anio);
        if (strlen(anio) > 0 && (!es_numero(anio) || v.anio < 1900 || v.anio > 2100))
            mostrar_error("El anio debe ser un numero entre 1900 y 2100");
        else
            break;
    } while (1);

    leer_texto(11, "Color", v.color, TAM_COLOR - 1);

    sem_wait(turno);
    memoria->operacion = ALTA_VEHICULO;
    memoria->vehiculo = v;
    enviar_peticion();
    correcto = memoria->correcto;
    strcpy(mensaje, memoria->mensaje);
    sem_post(turno);

    mostrar_resultado(correcto, mensaje);
}

void ver_vehiculos(void)
{
    Vehiculo lista[MAX_REGISTROS];
    Propietario propietarios[MAX_REGISTROS];
    int total;

    sem_wait(turno);
    memoria->operacion = LISTA_VEHICULOS;
    enviar_peticion();
    total = memoria->total;
    memcpy(lista, memoria->vehiculos, total * sizeof(Vehiculo));
    memcpy(propietarios, memoria->propietarios, sizeof(propietarios));
    sem_post(turno);

    titulo("Lista de vehiculos");
    attron(A_UNDERLINE);
    mvprintw(4, 2, "%-11s %-12s %-12s %-12s %-5s %-10s %-25s", "PLACA", "TIPO", "MARCA", "MODELO", "ANIO", "COLOR", "PROPIETARIO");
    attroff(A_UNDERLINE);

    if (total == 0)
        mvprintw(6, 2, "No hay vehiculos registrados");
    for (int i = 0; i < total && 5 + i < LINES - 4; i++) {
        char anio[6] = "-";
        char dueno[2 * TAM_NOMBRE];
        Propietario p = propietarios[lista[i].id_propietario - 1];
        if (lista[i].anio > 0)
            sprintf(anio, "%d", lista[i].anio);
        sprintf(dueno, "%s %s", p.nombre, p.apellidos);
        mvprintw(5 + i, 2, "%-11s %-12s %-12s %-12s %-5s %-10s %-25s", lista[i].placa, tipos_vehiculo[lista[i].tipo],
                 lista[i].marca, lista[i].modelo, anio, lista[i].color, dueno);
    }
    mvprintw(LINES - 2, 2, "Total: %d. Presiona una tecla para regresar...", total);
    getch();
}

// ---------- menus ----------

int menu(const char *nombre, const char *opciones[], int n)
{
    titulo(nombre);
    for (int i = 0; i < n; i++)
        mvprintw(4 + i * 2, 4, "%d) %s", i + 1, opciones[i]);
    mvprintw(5 + n * 2, 4, "Elige una opcion: ");
    refresh();

    int tecla;
    do {
        tecla = getch();
    } while (tecla < '1' || tecla >= '1' + n);
    return tecla - '1';
}

void salir(int senal)
{
    endwin();
    exit(0);
}

int main(void)
{
    if (abrir_memoria() == -1) {
        printf("No se pudo conectar con el servidor. Primero ejecuta ./servidor\n");
        return 1;
    }

    setlocale(LC_ALL, "");
    signal(SIGINT, salir);
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    const char *principal[] = { "Propietarios", "Vehiculos", "Control de acceso", "Salir" };
    const char *propietarios[] = { "Registrar propietario", "Ver propietarios", "Regresar" };
    const char *vehiculos[] = { "Registrar vehiculo", "Ver vehiculos", "Regresar" };
    int opcion;

    do {
        opcion = menu("Menu principal", principal, 4);
        if (opcion == 0) {
            int op;
            while ((op = menu("Propietarios", propietarios, 3)) != 2) {
                if (op == 0)
                    registrar_propietario();
                else
                    ver_propietarios();
            }
        } else if (opcion == 1) {
            int op;
            while ((op = menu("Vehiculos", vehiculos, 3)) != 2) {
                if (op == 0)
                    registrar_vehiculo();
                else
                    ver_vehiculos();
            }
        } else if (opcion == 2) {
            titulo("Control de acceso");
            mvprintw(4, 2, "Este modulo se va a agregar en la siguiente fase.");
            mvprintw(6, 2, "Presiona una tecla para regresar...");
            getch();
        }
    } while (opcion != 3);

    endwin();
    cerrar_memoria();
    return 0;
}
