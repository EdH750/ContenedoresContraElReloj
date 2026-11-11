#ifndef PILA_H_INCLUDED
#define PILA_H_INCLUDED

#include "nodo.h"

typedef tNodo* tPila;

void crear_pila(tPila* pp);
void vaciar_pila(tPila* pp);
int apilar(tPila* pp, const void* elem, unsigned tamElem);
int desapilar(tPila* pp, void* elem, unsigned tamElem);
int ver_tope(const tPila* pp, void* elem, unsigned tamElem);
int pila_llena(const tPila* pp, unsigned tamElem);
int pila_vacia(const tPila* pp);

#endif /* PILA_H_INCLUDED */
