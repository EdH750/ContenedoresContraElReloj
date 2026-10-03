#ifndef CONTENEDORESCONTRARELOJ_H_INCLUDED
#define CONTENEDORESCONTRARELOJ_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONFIG_ARCHIVO "config.txt"
#define PUERTO_ARCHIVO "puerto.txt"

#define MAX_BUFFER 100
#define MAX_COD_CONTENEDOR 12

#define MINIMO_TIEMPO_JORNADA 10
#define MAXIMO_TIEMPO_JORNADA 100

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
int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion);
void parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo);
int validar_campos(tConfiguracion* pConfiguracion);
void generar_archivo_puerto(tConfiguracion* pConfiguracion);
void ordenar_tiempos(int* tiempos, int cantidad);

#endif // CONTENEDORESCONTRARELOJ_H_INCLUDED
