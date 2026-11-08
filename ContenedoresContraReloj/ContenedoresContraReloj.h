#ifndef CONTENEDORESCONTRARELOJ_H_INCLUDED
#define CONTENEDORESCONTRARELOJ_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#define CONFIG_ARCHIVO "config.txt"
#define PUERTO_ARCHIVO "puerto.txt"

#define MAX_BUFFER 100

#define OPCION_MINIMA 1
#define OPCION_MAXIMA 4
#define OPCION_SALIR  4
#define OPCION_INVALIDA 0
#define MAX_COD_CONTENEDOR 12

#define MINIMO_TIEMPO_JORNADA 10
#define MAXIMO_TIEMPO_JORNADA 100

#define CANTIDAD_CAMPOS_CONFIG 10

/* Los codigos se arman como "C%d%02d": con 100 o mas contenedores por
   buque se repiten (C1101 = buque 1 cont. 101 = buque 11 cont. 1) */
#define MAXIMO_CONTENEDORES_POR_BUQUE 99

/* Coincide con los ids "B%03d" y "K%03d" (B999, K999) */
#define MAXIMO_BUQUES   999
#define MAXIMO_CAMIONES 999

typedef struct {

    int duracionJornadaMinutos;
    int cantidadMuelles;
    int cantidadZonasAlmacenamiento;
    int capacidadPila;
    int maximoBuques;
    int maximoContenedoresPorBuque;
    int maximoCamiones;
    int tiempoDescargaContenedor;
    int tiempoReubicacionContenedor;
    int tiempoCargaCamion;

}tConfiguracion;

void menu();
void seleccionar_opcion(int* opcion);
int validar_opcion(const char* texto, int* opcion);
int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion);
int linea_vacia(const char* linea);
int parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo);
int leer_entero(const char* texto, int* valor);
int validar_rango(int campo, int valor, int minimo, int maximo);
int validar_campos(tConfiguracion* pConfiguracion);
int generar_archivo_puerto(tConfiguracion* pConfiguracion);
void ordenar_tiempos(int* tiempos, int cantidad);

#endif // CONTENEDORESCONTRARELOJ_H_INCLUDED
