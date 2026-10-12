#ifndef OPEREACIONESYCOMANDOS_H_INCLUDED
#define OPEREACIONESYCOMANDOS_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include "../TDA/cola.h"
#include "../TDA/pila.h"
#include "../TDA/Estructuras.h"
#include "../TDA/lista.h"

    /*----------------------*/
    /*  DEFINE PARA ENT     */
    /*----------------------*/
#define CONTENEDOR_ENTREGADO 1
#define CONTENEDOR_FALTANTE 0
#define NO_RECUPERAR 0
#define COLA_CAMIONES_VACIA 2
#define LISTA_ZONAS_VACIA 3

    /*----------------------*/
    /*  DEFINE PARA DES     */
    /*----------------------*/
#define LISTA_MUELLES_VACIA 0
#define MUELLE_NO_VALIDO 2
#define ZONA_NO_VALIDA 3
#define ZONA_LLENA 4
#define MUELLE_DESCARGADO 1

#define SIN_TIEMPO 6


int operacionENT(tJornada* jornada);
int operacionDES(tJornada* jornada, int muelle, int zonaDestino);

int cmpZona(void* a, void* b);
int cmpMuelle(void*a, void*b);

int cmpContenedores(void* a, void* b);
void liberarEspacioZona(void* a);


#endif // OPEREACIONESYCOMANDOS_H_INCLUDED
