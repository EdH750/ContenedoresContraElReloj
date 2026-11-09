#include "lista.h"

void crear_lista(tLista* pl) {

    *pl = NULL;

}

void vaciar_lista(tLista* pl) {

    tNodo* aux;

    while(*pl) {

        aux = *pl;
        *pl = aux->sig;
        free(aux->info);
        free(aux);

    }

}

int enlistar_final(tLista* pl, const void* elem, unsigned tamElem) {

    tNodo* nue;

    while(*pl) {
        pl = &(*pl)->sig;
    }

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return 0;
    }

    memcpy(nue->info, elem, tamElem);
    nue->tamInfo = tamElem;
    nue->sig = NULL;

    *pl = nue;

    return 1;
}

int desenlistar_inicio(tLista* pl, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(*pl == NULL) {
        return 0;
    }

    aux = *pl;
    *pl = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    free(aux->info);
    free(aux);

    return 1;
}

int ver_primero_lista(const tLista* pl, void* elem, unsigned tamElem) {

    if(*pl == NULL) {
        return 0;
    }

    memcpy(elem, (*pl)->info, MINIMO(tamElem, (*pl)->tamInfo));

    return 1;
}

int lista_llena(const tLista* pl, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

int lista_vacia(const tLista* pl) {

    return *pl == NULL;

}

void recorrer_lista(tLista* pl, void (*accion)(void*)) {

    while(*pl) {
        accion((*pl)->info);
        pl = &(*pl)->sig;
    }
}

void* buscar_en_lista(tLista* pl, const void* clave, int (*cmp)(const void*, const void*)) {

    tNodo* aux;

    aux = *pl;

    while(aux) {
        if(cmp(aux->info, clave) == 0) {
            return aux->info;
        }
        aux = aux->sig;
    }

    return NULL;
}

