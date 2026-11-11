#include "cola.h"

/*
Inicializa la cola como vacia.
*/
void crear_cola(tCola* pc) {

    pc->pri = NULL;
    pc->ult = NULL;
}

/*
Libera todos los nodos de la cola y la deja vacia.
*/
void vaciar_cola(tCola* pc) {

    tNodo* aux;

    while(pc->pri) {
        aux = pc->pri;
        pc->pri = aux->sig;
        free(aux->info);
        free(aux);
    }

    pc->ult = NULL;
}

/*
Encola una copia de tamElem bytes del elemento apuntado por elem.
Devuelve EXITO si se pudo alojar memoria, ERROR en caso contrario.
*/
int encolar(tCola* pc, const void* elem, unsigned tamElem) {

    tNodo* nue;

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return ERROR;
    }

    memcpy(nue->info, elem, tamElem);
    nue->tamInfo = tamElem;
    nue->sig = NULL;

    if(pc->ult != NULL) {
        pc->ult->sig = nue;
    } else {
        pc->pri = nue;
    }

    pc->ult = nue;

    return EXITO;
}

/*
Desencola el primer elemento y copia su contenido en elem (hasta tamElem bytes).
Devuelve EXITO si habia elemento, ERROR si la cola estaba vacia.
*/
int desencolar(tCola* pc, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(pc->pri == NULL) {
        return ERROR;
    }

    aux = pc->pri;
    pc->pri = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    if(pc->pri == NULL) {
        pc->ult = NULL;
    }

    free(aux->info);
    free(aux);

    return EXITO;
}

/*
Copia el primer elemento en elem sin desencolarlo.
Devuelve EXITO si habia elemento, ERROR si la cola estaba vacia.
*/
int ver_primero_cola(const tCola* pc, void* elem, unsigned tamElem) {

    if(pc->pri == NULL) {
        return ERROR;
    }

    memcpy(elem, pc->pri->info, MINIMO(tamElem, pc->pri->tamInfo));

    return EXITO;
}

/*
Devuelve distinto de cero si no hay memoria suficiente para encolar un elemento de tamElem bytes.
*/
int cola_llena(const tCola* pc, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux  = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

/*
Devuelve distinto de cero si la cola no tiene elementos.
*/
int cola_vacia(const tCola* pc) {

    return pc->pri == NULL;
}
