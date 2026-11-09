#ifndef COLA_H_INCLUDED
#define COLA_H_INCLUDED

#include "nodo.h"

typedef struct {

    tNodo* pri;
    tNodo* ult;

}tCola;

void crear_cola(tCola* pc);
void vaciar_cola(tCola* pc);
int encolar(tCola* pc, const void* elem, unsigned tamElem);
int desencolar(tCola* pc, void* elem, unsigned tamElem);
int ver_primero_cola(const tCola* pc, void* elem, unsigned tamElem);
int cola_llena(const tCola* pc, unsigned tamElem);
int cola_vacia(const tCola* pc);

#endif /* COLA_H_INCLUDED */
