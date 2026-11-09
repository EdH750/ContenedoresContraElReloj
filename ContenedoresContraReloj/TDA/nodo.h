#ifndef NODO_H_INCLUDED
#define NODO_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#define MINIMO(X,Y) ((X)<(Y)?(X):(Y))

typedef struct sNodo {

    void* info;
    unsigned tamInfo;
    struct sNodo* sig;

}tNodo;

#endif /* NODO_H_INCLUDED */
