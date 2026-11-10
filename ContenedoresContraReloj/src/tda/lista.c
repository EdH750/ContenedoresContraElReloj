#include "lista.h"

/*
Inicializa la lista como vacia.
*/
void crear_lista(tLista* pl) {

    *pl = NULL;
}

/*
Libera todos los nodos de la lista y la deja vacia.
*/
void vaciar_lista(tLista* pl) {

    tNodo* aux;

    while(*pl) {
        aux = *pl;
        *pl = aux->sig;
        free(aux->info);
        free(aux);
    }
}

/*
Agrega al final de la lista una copia de tamElem bytes del elemento apuntado por elem.
Devuelve EXITO si se pudo alojar memoria, ERROR en caso contrario.
*/
int enlistar_final(tLista* pl, const void* elem, unsigned tamElem) {

    tNodo* nue;

    while(*pl) {
        pl = &(*pl)->sig;
    }

    if((nue = (tNodo*)malloc(sizeof(tNodo))) == NULL || (nue->info = malloc(tamElem)) == NULL) {
        free(nue);
        return ERROR;
    }

    memcpy(nue->info, elem, tamElem);
    nue->tamInfo = tamElem;
    nue->sig = NULL;
    *pl = nue;

    return EXITO;
}

/*
Elimina el primer elemento de la lista y copia su contenido en elem (hasta tamElem bytes).
Devuelve EXITO si habia elemento, ERROR si la lista estaba vacia.
*/
int desenlistar_inicio(tLista* pl, void* elem, unsigned tamElem) {

    tNodo* aux;

    if(*pl == NULL) {
        return ERROR;
    }

    aux = *pl;
    *pl = aux->sig;

    memcpy(elem, aux->info, MINIMO(tamElem, aux->tamInfo));

    free(aux->info);
    free(aux);

    return EXITO;
}

/*
Copia el primer elemento en elem sin eliminarlo.
Devuelve EXITO si habia elemento, ERROR si la lista estaba vacia.
*/
int ver_primero_lista(const tLista* pl, void* elem, unsigned tamElem) {

    if(*pl == NULL) {
        return ERROR;
    }

    memcpy(elem, (*pl)->info, MINIMO(tamElem, (*pl)->tamInfo));

    return EXITO;
}

/*
Devuelve distinto de cero si no hay memoria suficiente para enlistar un elemento de tamElem bytes.
*/
int lista_llena(const tLista* pl, unsigned tamElem) {

    tNodo* aux;
    void* elem;

    aux  = (tNodo*)malloc(sizeof(tNodo));
    elem = malloc(tamElem);

    free(aux);
    free(elem);

    return aux == NULL || elem == NULL;
}

/*
Devuelve distinto de cero si la lista no tiene elementos.
*/
int lista_vacia(const tLista* pl) {

    return *pl == NULL;
}

/*
Ordena la lista en el orden definido por cmp usando insercion.
*/
void ordenar_lista(tLista* pl, int (*cmp)(const void*, const void*)) {

    tLista* pri, *listaAux;
    tNodo* nodoAux;

    pri = pl;

    if(*pl == NULL) {
        return;
    }

    while((*pl)->sig) {

        if(cmp((*pl)->info, (*pl)->sig->info) > 0) {

            nodoAux = (*pl)->sig;
            (*pl)->sig = nodoAux->sig;
            listaAux = pri;

            while(cmp((*listaAux)->info, nodoAux->info) <= 0) {
                listaAux = &(*listaAux)->sig;
            }

            nodoAux->sig = *listaAux;
            *listaAux = nodoAux;

        } else {

            pl = &(*pl)->sig;
        }
    }
}

/*
Recorre la lista llamando accion(info) en cada nodo.
*/
void recorrer_lista(tLista* pl, void (*accion)(void*)) {

    while(*pl) {
        accion((*pl)->info);
        pl = &(*pl)->sig;
    }
}

/*
Busca en la lista el primer elemento tal que cmp(elemento, clave) == 0.
Devuelve un puntero al dato del nodo si lo encuentra, NULL si no esta.
*/
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
