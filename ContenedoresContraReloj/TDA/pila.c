#include "pila.h"

void CrearPila(tPila* p)
{
    *p=NULL;
}

int PilaLlena(const tPila* p, unsigned tam)
{
    tNodo *aux = (tNodo*)malloc(sizeof(tNodo));
    void *info = malloc(tam);

    free(aux);
    free(info);

    return aux == NULL || info == NULL;
}

int PonerEnPila(tPila* p, unsigned tam, void* dato)
{
    tNodo* nuevo;
    nuevo = (tNodo*)malloc(sizeof(tNodo));

    if(nuevo == NULL)
        return SIN_MEM;

    nuevo->info = malloc(tam);

    if(nuevo->info == NULL)
    {
        free(nuevo);
        return SIN_MEM;
    }

    memcpy(nuevo->info, dato, tam);
    nuevo->tamInfo = tam;
    nuevo->sig = *p;
    *p = nuevo;

    return TODO_OK;
}

int VerTope(const tPila* p, unsigned tam, void* dato)
{
    if(*p == NULL)
        return PILA_VACIA;

    memcpy(dato, (*p)->info, MINIMO(tam, (*p)->tamInfo));
    return TODO_OK;
}

int PilaVacia(const tPila* p)
{
    return *p == NULL;
}

int SacarDePila(tPila* p, unsigned tam, void* dato)
{
    tNodo* aux = *p;

    if(aux == NULL)
        return PILA_VACIA;
    *p = aux->sig;
    memcpy(dato, aux->info, MINIMO(tam, aux->tamInfo));
    free(aux->info);
    free(aux);

    return TODO_OK;
}

void VaciarPila(tPila* p)
{
    while(*p != NULL)
    {
        tNodo* aux = *p;
        *p = aux->sig;
        free(aux->info);
        free(aux);
    }
}
