#ifndef NODO_H_INCLUDED
#define NODO_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#define MINIMO(X,Y) ((X)<(Y)?(X):(Y))

#define EXITO 1   /* retorno que indica operacion exitosa */
#define ERROR 0   /* retorno que indica fallo o ausencia */

typedef struct sNodo {

    void* info;
    unsigned tamInfo;
    struct sNodo* sig;

}tNodo;

#endif /* NODO_H_INCLUDED */
