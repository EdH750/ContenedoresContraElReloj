#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include "util.h"

#define OPCION_MINIMA     1
#define OPCION_MAXIMA     3
#define OPCION_SALIR      3
#define OPCION_INVALIDA   0

/* Imprime las opciones del menu principal en pantalla */
void menu(void);

/* Lee una linea del teclado, la valida y deja el numero elegido en *opcion.
   Si la entrada es invalida deja OPCION_INVALIDA para que el switch en main
   caiga en el default. Si fgets devuelve NULL (EOF) deja OPCION_SALIR */
void seleccionar_opcion(int* opcion);

/* Verifica que 'texto' represente un entero valido dentro del rango de opciones.
   Devuelve EXITO si es valido y deja el valor en *opcion.
   Devuelve ERROR en cualquier otro caso (no modifica *opcion en ese caso) */
int validar_opcion(const char* texto, int* opcion);

#endif /* MENU_H_INCLUDED */
