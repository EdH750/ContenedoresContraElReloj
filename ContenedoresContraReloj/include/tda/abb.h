#ifndef ABB_H_INCLUDED
#define ABB_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nodo.h"

/* Codigos de retorno de insertar_abb.
   EXITO (1) e ERROR (0) vienen de nodo.h.
   ABB_CLAVE_DUP es un tercer estado: la clave ya existia, no es un error */
#define ABB_CLAVE_DUP 2   /* clave duplicada: no se inserta, pero no es un fallo */

/* Nodo interno del arbol.
   info apunta a una copia del dato (tamInfo bytes) hecha con malloc.
   El comparador que recibe cada operacion define el orden del arbol */
typedef struct sNodoABB {
    void*            info;
    size_t           tamInfo;
    struct sNodoABB* izq;
    struct sNodoABB* der;
} tNodoABB;

/* El arbol se representa como un puntero a su raiz.
   Un arbol vacio es NULL */
typedef tNodoABB* tABB;

/* Crea (inicializa) un arbol vacio */
void crear_abb(tABB* pa);

/* Libera todos los nodos en postorden y deja el arbol vacio */
void vaciar_abb(tABB* pa);

/* Inserta una copia de pd (tam bytes) usando cmp para ubicarla.
   Devuelve EXITO si se inserto, ABB_CLAVE_DUP si ya existia, ERROR si fallo malloc */
int insertar_abb(tABB* pa, const void* pd, size_t tam,
                 int (*cmp)(const void*, const void*));

/* Busca la clave en el arbol usando cmp.
   Devuelve un puntero al dato del nodo si lo encuentra, NULL si no */
void* buscar_abb(tABB* pa, const void* clave,
                 int (*cmp)(const void*, const void*));

/* Recorre el arbol en inorden (izq -> raiz -> der), llamando accion(info)
   en cada nodo. Produce los elementos en el orden definido por cmp */
void recorrer_abb_en_orden(tABB* pa, void (*accion)(void*));

#endif /* ABB_H_INCLUDED */
