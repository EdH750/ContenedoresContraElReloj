#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include "nodo.h"

typedef tNodo* tLista;

void crear_lista(tLista* pl);
void vaciar_lista(tLista* pl);
int enlistar_final(tLista* pl, const void* elem, unsigned tamElem);
int desenlistar_inicio(tLista* pl, void* elem, unsigned tamElem);
int ver_primero_lista(const tLista* pl, void* elem, unsigned tamElem);
int lista_llena(const tLista* pl, unsigned tamElem);
int lista_vacia(const tLista* pl);

#endif /* LISTA_H_INCLUDED */
