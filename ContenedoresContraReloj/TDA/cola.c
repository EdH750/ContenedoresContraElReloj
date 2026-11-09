#include "cola.h"

void crear_cola(tCola* pc) {

    pc->pri = NULL;
    pc->ult = NULL;

}

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

int encolar(tCola* pc, const void* elem, unsigned tamElem) {

    tNodo* nue;

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return 0;
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

    return 1;
}

int desencolar(tCola* pc, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(pc->pri == NULL) {
        return 0;
    }

    aux = pc->pri;
    pc->pri = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    if(pc->pri == NULL) {
        pc->ult = NULL;
    }

    free(aux->info);
    free(aux);

    return 1;
}

int ver_primero_cola(const tCola* pc, void* elem, unsigned tamElem) {

    if(pc->pri == NULL) {
        return 0;
    }

    memcpy(elem, pc->pri->info, MINIMO(tamElem, pc->pri->tamInfo));

    return 1;
}

int cola_llena(const tCola* pc, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

int cola_vacia(const tCola* pc) {

    return pc->pri == NULL;

}


