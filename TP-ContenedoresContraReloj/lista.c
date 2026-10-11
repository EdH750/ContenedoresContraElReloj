#include "lista.h"

void CrearLista(tLista* l)
{
    *l = NULL;
}

int ListaVacia(const tLista* l)
{
    return *l == NULL;
}

int ListaLlena(const tLista* l, unsigned tam)
{
    tNodo* aux = (tNodo*)malloc(sizeof(tNodo));
    if(aux == NULL)
        return ERROR_MEM;
    void *info = malloc(tam);
    if(info == NULL)
    {
        free(aux);
        return ERROR_MEM;
    }
    free(aux);
    free(info);

    return aux == NULL || info == NULL;
}

void VaciarLista(tLista* l)
{
    while(*l != NULL)
    {
        tNodo *aux = *l;
        *l = aux->sig;
        free(aux->info);
        free(aux);
    }
}

int PonerAlComienzo(tLista* l, unsigned tam, const void* dato)
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
    nuevo->sig = *l;
    *l = nuevo;
    return OK;
}

int SacarPrimeroLista(tLista* l, unsigned tam, void* dato)
{
    tNodo* aux = *l;

    if(aux == NULL)
        return LISTA_VACIA;

    *l = aux->sig;
    memcpy(dato, aux->info, MIN(tam, aux->tamInfo));
    free(aux->info);
    free(aux);
    return OK;
}

int VerPrimeroLista(const tLista* l, unsigned tam, void* dato)
{
    if(*l == NULL)
        return LISTA_VACIA;
    memcpy(dato, (*l)->info, MIN(tam, (*l)->tamInfo));
    return OK;
}

int PonerAlFinal(tLista* l, unsigned tam, const void* dato)
{
    tNodo* nuevo;

    while(*l != NULL)
        l = &(*l)->sig;
    nuevo = (tNodo*)malloc(sizeof(tNodo));
    if(nuevo == NULL)
        return ERROR_MEM;
    nuevo->info=malloc(tam);
    if(nuevo->info == NULL)
    {
        free(nuevo);
        return ERROR_MEM;
    }
    memcpy(nuevo->info, dato, tam);
    nuevo->tamInfo = tam;
    nuevo->sig = NULL;
    *l = nuevo;
    return OK;
}

int SacarUltimoLista(tLista* l, unsigned tam, void* dato)
{
    if(*l == NULL)
        return LISTA_VACIA;

    while((*l)->sig)
        l=&(*l)->sig;
    memcpy(dato, (*l)->info, MIN(tam, (*l)->tamInfo));
    return OK;
}

int VerUltimoLista(const tLista* l, unsigned tam, void* dato)
{
    if(*l == NULL)
        return LISTA_VACIA;
    while((*l)->sig)
        l=&(*l)->sig;
    memcpy(dato, (*l)->info, MIN(tam, (*l)->tamInfo));
    return OK;
}
