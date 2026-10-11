#include "cola.h"

void CrearCola(tCola* c)
{
    c->primero = NULL;
    c->ultimo = NULL;
}

int ColaLlena(const tCola* c, unsigned tam)
{
    tNodo* aux = (tNodo*)malloc(sizeof(tNodo));

    if(aux==NULL)
        return ERROR_MEM;

    void* info = malloc(tam);

    if(info == NULL)
    {
        free(aux);
        return ERROR_MEM;
    }

    return aux == NULL || info == NULL;
}

int PonerEnCola(tCola* c, unsigned tam, const void* dato)
{
    tNodo* nuevo = (tNodo*)malloc(sizeof(tNodo));

    if(nuevo == NULL)
        return ERROR_MEM;
    nuevo->info = malloc(tam);
    if(nuevo->info == NULL)
    {
        free(nuevo);
        return ERROR_MEM;
    }
    memcpy(nuevo->info, dato, tam);
    nuevo->tamInfo = tam;
    nuevo->sig = NULL;
    if(c->ultimo)
        c->ultimo->sig = nuevo;
    else
        c->primero = nuevo;
    c->ultimo = nuevo;

    return OK;
}

int VerPrimeroEnCola(const tCola* c, unsigned tam, void* dato)
{
    if(c->primero == NULL)
        return COLA_VACIA;
    memcpy(dato, c->primero->info, MIN(tam, c->primero->tamInfo));
    return OK;
}

int ColaVacia(const tCola* c)
{
    return c->primero == NULL;
}

int SacarDeLaCola(tCola* c, unsigned tam, void* dato)
{
    tNodo* aux = c->primero;
    if(aux == NULL)
        return COLA_VACIA;
    c->primero = aux->sig;
    memcpy(dato, aux->info, MIN(aux->tamInfo, tam));
    free(aux->info);
    free(aux);
    if(c->primero == NULL)
        c->ultimo = NULL;
    return OK;
}

void VaciarCola(tCola* c)
{
    while(c->primero != NULL)
    {
        tNodo* aux = c->primero;
        c->primero = aux->sig;
        free(aux->info);
        free(aux);
    }
    c->ultimo = NULL;
}
