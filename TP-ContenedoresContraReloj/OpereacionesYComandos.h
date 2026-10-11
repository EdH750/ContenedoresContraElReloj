#ifndef OPEREACIONESYCOMANDOS_H_INCLUDED
#define OPEREACIONESYCOMANDOS_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include "cola.h"
#include "pila.h"
#include "Estructuras.h"
#include "lista.h"

    /*----------------------*/
    /*  DEFINE PARA ENT     */
    /*----------------------*/
#define CONTENEDOR_ENTREGADO 1
#define CONTENEDOR_FALTANTE 0
#define COLA_CAMIONES_VACIA 2
#define LISTA_ZONAS_VACIA 3

#define SIN_TIEMPO 6


int operacionENT(tJornada* jornada);
int operacionDES(tJornada* jornada);

int cmpContenedores(void* a, void* b);
void liberarEspacioZona(void* a);


#endif // OPEREACIONESYCOMANDOS_H_INCLUDED
