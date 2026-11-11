#ifndef APLICACION_H_INCLUDED
#define APLICACION_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include "util.h"

#define CONFIG_ARCHIVO "config.txt"
#define PUERTO_ARCHIVO "puerto.txt"

#define MAX_COD_CONTENEDOR 12

#define MINIMO_TIEMPO_JORNADA 10
#define MAXIMO_TIEMPO_JORNADA 100

#define CANTIDAD_CAMPOS_CONFIG        10
#define MINIMO_CANTIDAD               1
#define PRIMER_CAMPO                  1

#define MAXIMO_MUELLES                20
#define MAXIMO_ZONAS                  20
#define MAXIMO_CAPACIDAD_PILA         50
#define MAXIMO_CONTENEDORES_POR_BUQUE 99
#define MAXIMO_BUQUES                 999
#define MAXIMO_CAMIONES               999

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

} tConfiguracion;

int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion);
int linea_vacia(const char* linea);
int parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo);
int validar_rango(int campo, int valor, int minimo, int maximo);
int validar_campos(tConfiguracion* pConfiguracion);
int generar_archivo_puerto(tConfiguracion* pConfiguracion);
void ordenar_tiempos(int* tiempos, int cantidad);

#endif /* APLICACION_H_INCLUDED */
