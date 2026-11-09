#ifndef ABB_H_INCLUDED
#define ABB_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Codigos de retorno de insertar_abb */
#define ABB_OK        1   /* insercion exitosa */
#define ABB_CLAVE_DUP 0   /* clave ya existia: no se inserta */
#define ABB_SIN_MEM  -1   /* malloc fallo */

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
   Devuelve ABB_OK, ABB_CLAVE_DUP o ABB_SIN_MEM */
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
