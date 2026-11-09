#include "abb.h"

/* Un arbol vacio es NULL: crear es solo asignar NULL */
void crear_abb(tABB* pa) {
    *pa = NULL;
}

/* Liberacion en postorden: primero los subarboles, luego el nodo actual.
   Cada nodo tiene dos mallocs (el nodo + su info), ambos se liberan aqui */
void vaciar_abb(tABB* pa) {
    if(!*pa) return;

    /* Primero los hijos (postorden garantiza que no queden nodos huerfanos) */
    vaciar_abb(&(*pa)->izq);
    vaciar_abb(&(*pa)->der);

    /* Luego el nodo actual */
    free((*pa)->info);
    free(*pa);
    *pa = NULL;
}

/* Insercion recursiva. El arbol queda ordenado por el criterio de cmp:
   cmp(pd, nodo->info) < 0 -> va a la izquierda
   cmp(pd, nodo->info) > 0 -> va a la derecha
   cmp(pd, nodo->info) == 0 -> clave duplicada, no se inserta */
int insertar_abb(tABB* pa, const void* pd, size_t tam,
                 int (*cmp)(const void*, const void*)) {

    tNodoABB* nue;
    int res;

    if(*pa) {
        /* El arbol no esta vacio: buscar el lugar correcto */
        res = cmp(pd, (*pa)->info);
        if(res < 0) return insertar_abb(&(*pa)->izq, pd, tam, cmp);
        if(res > 0) return insertar_abb(&(*pa)->der, pd, tam, cmp);
        return ABB_CLAVE_DUP;
    }

    /* Posicion libre: crear el nodo */
    nue = (tNodoABB*)malloc(sizeof(tNodoABB));
    if(!nue) return ABB_SIN_MEM;

    nue->info = malloc(tam);
    if(!nue->info) {
        free(nue);
        return ABB_SIN_MEM;
    }

    memcpy(nue->info, pd, tam);
    nue->tamInfo = tam;
    nue->izq     = NULL;
    nue->der     = NULL;

    *pa = nue;
    return ABB_OK;
}

/* Busqueda iterativa (mas eficiente que la recursiva en arboles profundos).
   Recorre el camino desde la raiz hasta encontrar la clave o llegar a NULL */
void* buscar_abb(tABB* pa, const void* clave,
                 int (*cmp)(const void*, const void*)) {

    int res;

    while(*pa) {
        res = cmp(clave, (*pa)->info);
        if(res < 0)      pa = &(*pa)->izq;
        else if(res > 0) pa = &(*pa)->der;
        else             return (*pa)->info;   /* encontrado: devolver puntero al dato */
    }

    return NULL;   /* no esta en el arbol */
}

/* Recorrido en inorden (izq -> raiz -> der).
   En un ABB, el inorden produce los elementos en orden ascendente segun cmp */
void recorrer_abb_en_orden(tABB* pa, void (*accion)(void*)) {
    if(!*pa) return;
    recorrer_abb_en_orden(&(*pa)->izq, accion);
    accion((*pa)->info);
    recorrer_abb_en_orden(&(*pa)->der, accion);
}
