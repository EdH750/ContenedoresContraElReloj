#ifndef COLA_H_INCLUDED
#define COLA_H_INCLUDED

#define MIN(X,Y) ((X)<=(Y)?(X):(Y))
#define ERROR_MEM 2
#define OK 3
#define COLA_VACIA 4

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Estructuras.h"

void CrearCola(tCola* c);
int ColaLlena(const tCola* c, unsigned tam);
int PonerEnCola(tCola* c, unsigned tam, const void* dato);
int VerPrimeroEnCola(const tCola* c, unsigned tam, void* dato);
int ColaVacia(const tCola* c);
int SacarDeLaCola(tCola* c, unsigned tam, void* dato);
void VaciarCola(tCola* c);

#endif // COLA_H_INCLUDED
