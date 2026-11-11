#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#define BASE_DECIMAL 10   /* base decimal para strtol */
#define MAX_BUFFER   100  /* tamaño maximo de un buffer de lectura de linea */

/* EXITO/ERROR pueden venir de nodo.h si se incluye algun TDA; se definen
   aqui con guarda para que los modulos que no incluyen TDAs tambien los tengan */
#ifndef EXITO
#define EXITO 1
#define ERROR 0
#endif

/* Convierte una cadena a int de forma estricta usando strtol.
   A diferencia de atoi, detecta todos los casos invalidos:
     - cadena vacia ("")
     - solo letras ("abc")
     - numero con basura al final ("5abc", "2.7")
     - numero fuera del rango de int
   Devuelve EXITO si pudo convertir, ERROR en cualquier otro caso */
int leer_entero(const char* texto, int* valor);

#endif /* UTIL_H_INCLUDED */
