#include "abb.h"

/*
Inicializa el arbol como vacio.
*/
void crear_abb(tABB* pa) {

    *pa = NULL;
}

/*
Libera todos los nodos del arbol en postorden y lo deja vacio.
*/
void vaciar_abb(tABB* pa) {

    if(!*pa) return;

    vaciar_abb(&(*pa)->izq);
    vaciar_abb(&(*pa)->der);

    free((*pa)->info);
    free(*pa);
    *pa = NULL;
}

/*
Inserta una copia de pd (tam bytes) en el arbol usando cmp para ubicarla.
Devuelve EXITO si se inserto, ABB_CLAVE_DUP si ya existia, ERROR si fallo malloc.
*/
int insertar_abb(tABB* pa, const void* pd, size_t tam,
                 int (*cmp)(const void*, const void*)) {

    tNodoABB* nue;
    int res;

    if(*pa) {
        res = cmp(pd, (*pa)->info);
        if(res < 0) return insertar_abb(&(*pa)->izq, pd, tam, cmp);
        if(res > 0) return insertar_abb(&(*pa)->der, pd, tam, cmp);
        return ABB_CLAVE_DUP;
    }

    nue = (tNodoABB*)malloc(sizeof(tNodoABB));
    if(!nue) return ERROR;

    nue->info = malloc(tam);
    if(!nue->info) {
        free(nue);
        return ERROR;
    }

    memcpy(nue->info, pd, tam);
    nue->tamInfo = tam;
    nue->izq     = NULL;
    nue->der     = NULL;

    *pa = nue;

    return EXITO;
}

/*
Busca la clave en el arbol usando cmp de forma iterativa.
Devuelve un puntero al dato del nodo si lo encuentra, NULL si no esta.
*/
void* buscar_abb(tABB* pa, const void* clave,
                 int (*cmp)(const void*, const void*)) {

    int res;

    while(*pa) {
        res = cmp(clave, (*pa)->info);
        if(res < 0)      pa = &(*pa)->izq;
        else if(res > 0) pa = &(*pa)->der;
        else             return (*pa)->info;
    }

    return NULL;
}

/*
Recorre el arbol en inorden llamando accion(info) en cada nodo.
Produce los elementos en el orden ascendente definido por cmp.
*/
void recorrer_abb_en_orden(tABB* pa, void (*accion)(void*)) {

    if(!*pa) return;
    recorrer_abb_en_orden(&(*pa)->izq, accion);
    accion((*pa)->info);
    recorrer_abb_en_orden(&(*pa)->der, accion);
}
