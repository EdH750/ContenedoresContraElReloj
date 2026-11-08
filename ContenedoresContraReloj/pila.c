#include "pila.h"

void crear_pila(tPila* pp) {

    *pp = NULL;

}

void vaciar_pila(tPila* pp) {

    tNodo* aux;

    while(*pp) {

        aux = *pp;
        *pp = aux->sig;
        free(aux->info);
        free(aux);

    }

}

int apilar(tPila* pp, const void* elem, unsigned tamElem) {

    tNodo* nue;

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return 0;
    }

    memcpy(nue->info, elem, tamElem);
    nue->tamInfo = tamElem;
    nue->sig = *pp;

    *pp = nue;

    return 1;
}

int desapilar(tPila* pp, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(*pp == NULL) {
        return 0;
    }

    aux = *pp;
    *pp = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    free(aux->info);
    free(aux);

    return 1;
}

int ver_tope(const tPila* pp, void* elem, unsigned tamElem) {

    if(*pp == NULL) {
        return 0;
    }

    memcpy(elem, (*pp)->info, MINIMO(tamElem, (*pp)->tamInfo));

    return 1;
}

int pila_llena(const tPila* pp, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

int pila_vacia(const tPila* pp) {

    return *pp == NULL;

}
