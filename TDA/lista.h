#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Estructuras.h"

#define MIN(X,Y) ((X)<=(Y)?(X):(Y))
#define ERROR_MEM 2
#define OK 3
#define LISTA_VACIA 4
#define NO_ENCONTRADO 0

void CrearLista(tLista* l);

int ListaVacia(const tLista* l);

int ListaLlena(const tLista* l, unsigned tam);

void VaciarLista(tLista* l);

int PonerAlComienzo(tLista* l, unsigned tam, const void* dato);

int SacarPrimeroLista(tLista* l, unsigned tam, void* dato);

int VerPrimeroLista(const tLista* l, unsigned tam, void* dato);

int PonerAlFinal(tLista* l, unsigned tam,const void* dato);

int SacarUltimoLista(tLista* l, unsigned tam, void* dato);

int VerUltimoLista(const tLista* l, unsigned tam, void* dato);

int buscarEnListaSinBorrar(tLista* pl, void* dato, unsigned tam,int(*comp)(void*,void*),void (*accion)(void*));
#endif // LISTA_H_INCLUDED
