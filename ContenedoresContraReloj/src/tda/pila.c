#include "pila.h"

/*
Inicializa la pila como vacia.
*/
void crear_pila(tPila* pp) {

    *pp = NULL;
}

/*
Libera todos los nodos de la pila y la deja vacia.
*/
void vaciar_pila(tPila* pp) {

    tNodo* aux;

    while(*pp) {
        aux = *pp;
        *pp = aux->sig;
        free(aux->info);
        free(aux);
    }
}

/*
Apila una copia de tamElem bytes del elemento apuntado por elem.
Devuelve EXITO si se pudo alojar memoria, ERROR en caso contrario.
*/
int apilar(tPila* pp, const void* elem, unsigned tamElem) {

    tNodo* nue;

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return ERROR;
    }

    memcpy(nue->info, elem, tamElem);
    nue->tamInfo = tamElem;
    nue->sig = *pp;
    *pp = nue;

    return EXITO;
}

/*
Desapila el tope y copia su contenido en elem (hasta tamElem bytes).
Devuelve EXITO si habia elemento, ERROR si la pila estaba vacia.
*/
int desapilar(tPila* pp, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(*pp == NULL) {
        return ERROR;
    }

    aux = *pp;
    *pp = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    free(aux->info);
    free(aux);

    return EXITO;
}

/*
Copia el tope en elem sin desapilarlo.
Devuelve EXITO si habia elemento, ERROR si la pila estaba vacia.
*/
int ver_tope(const tPila* pp, void* elem, unsigned tamElem) {

    if(*pp == NULL) {
        return ERROR;
    }

    memcpy(elem, (*pp)->info, MINIMO(tamElem, (*pp)->tamInfo));

    return EXITO;
}

/*
Devuelve distinto de cero si no hay memoria suficiente para apilar un elemento de tamElem bytes.
*/
int pila_llena(const tPila* pp, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux  = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

/*
Devuelve distinto de cero si la pila no tiene elementos.
*/
int pila_vacia(const tPila* pp) {

    return *pp == NULL;
}
