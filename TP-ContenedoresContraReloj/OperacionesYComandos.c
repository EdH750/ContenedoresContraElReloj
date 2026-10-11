#include "OpereacionesYComandos.h"

int OperacionREU(tJornada *jornada, int idOrigen, int idDestino)
{
    tZona *zonaOrigen = NULL;
    tZona *zonaDestino = NULL;
    tZona *zona;
    tNodo *actual;
    int T_inicio, T_fin;
    tContenedor contenedor;
    tOperacion operacion;

    actual = jornada->puerto.zonas;

    while(actual != NULL)
    {
        zona = (tZona*)actual->info;

        if(zona->id == idOrigen)
            zonaOrigen = zona;

        if(zona->id == idDestino)
            zonaDestino = zona;

        actual = actual->sig;
    }

    if(zonaOrigen == NULL || zonaDestino == NULL)
        return ZONA_INEXISTENTE;

    if(idOrigen == idDestino)
        return ZONAS_IGUALES;

    if(PilaVacia(&zonaOrigen->pila))
        return ZONA_VACIA;

    if(zonaDestino->cantidad >= jornada->cfg.capacidadPila)
        return ZONA_LLENA;

    T_inicio = jornada->tiempoActual;
    T_fin = T_inicio + jornada->cfg.tiempoReubicacionContenedor;

    if(T_fin > jornada->cfg.duracionJornadaMinutos)
        return SIN_TIEMPO;

    SacarDePila(&zonaOrigen->pila, sizeof(tContenedor), &contenedor);

    if(PonerEnPila(&zonaDestino->pila, sizeof(tContenedor), &contenedor) == SIN_MEM)
    {
        PonerEnPila(&zonaOrigen->pila, sizeof(tContenedor), &contenedor);
        return SIN_MEM;
    }

    zonaDestino->cantidad++;
    zonaOrigen->cantidad--;

    operacion.tInicio = T_inicio;
    operacion.tFin = T_fin;

    strcpy(operacion.codigo, "REU");

    operacion.contenedor = contenedor;
    ArmarCodigo(operacion.origen, "Z", idOrigen);
    ArmarCodigo(operacion.destino, "Z", idDestino);

    jornada->resumen.reubicaciones++;

    PonerAlFinal(&jornada->historial, sizeof(tOperacion), &operacion);

    jornada->tiempoActual = T_fin;

    return OK;
}

void ArmarCodigo (char *destino, char *letra, int id)
{
    int divisor = 1;
    int aux = id;

    *destino = *letra;
    destino++;

    while(aux >= 10)
    {
        aux = aux / 10;
        divisor = divisor * 10;
    }

    while(divisor > 0)
    {
        *destino = '0' + id / divisor;
        destino++;
        id = id % divisor;
        divisor = divisor / 10;
    }
    *destino = '\0';
}

void OperacionVER(tJornada *jornada)
{
    tPuerto *puerto = &jornada->puerto;
    tNodo *nodo;
    tZona *zona;
    tMuelle *muelle;
    tBuque *buque;
    int puntaje, i, pendientes, buquesEspera;
    char *prox;

    printf("\n ========== Estado del Puerto ==========\n");
    printf("\nTiempo Actual: %d", jornada->tiempoActual);

    printf("\n --- Muelles ---\n");

    for(i=0; i < jornada->puerto.cantMuelles; i++)
    {
        muelle = &puerto->muelles[i];

        if(muelle->buque == NULL)
            printf("M%d: libre\n", muelle->id);
        else
        {
            buque = muelle->buque;
            pendientes = buque->cantContenedores - buque->proxADescargar;

            if(pendientes > 0)
                prox = buque->manifiesto[buque->proxADescargar].codigo;
            else
                prox = "-";

            printf("M%d: %s | Pendientes: %d | Proximo: %s\n", muelle->id, buque->id, pendientes,prox);
        }
    }

    buquesEspera = ContarBuquesEnEspera(&puerto->colaBuques);
    printf("\nBuques en la cola de espera: %d\n", buquesEspera);

    printf("\n --- Zonas de Almacenamiento --- \n");

    nodo = puerto->zonas;
    while(nodo != NULL)
    {
        zona = (tZona*)nodo->info;

        printf("Z%d [%d/%d] ", zona->id, zona->cantidad, jornada->cfg.capacidadPila);
        ImprimirPilaContenedores(&zona->pila);
        printf("\n");
        nodo=nodo->sig;
    }

    printf("\n --- Camiones ---\n");
    ImprimirVentanaDePlanificacion(&puerto->colaCamiones);

    puntaje=jornada->resumen.contenedoresEntregados * 10 + jornada->resumen.buquesDescargados * 5;
    printf("\nPuntaje Provisorio: %d", puntaje);
}

int ContarBuquesEnEspera(tCola *cola)
{
    void *elem;
    tCola colaAux;
    int cant = 0;

    CrearCola(&colaAux);

    while(SacarDeLaCola(cola, sizeof(void*), &elem) != COLA_VACIA)
    {
        PonerEnCola(&colaAux, sizeof(void*), &elem);
        cant++;
    }
    while(SacarDeLaCola(&colaAux, sizeof(void*), &elem) != COLA_VACIA)
        PonerEnCola(cola, sizeof(void*), &elem);

    return cant;
}

void ImprimirPilaContenedores(tPila *pila)
{
    tPila pilaAux;
    tContenedor contenedor;
    int primero = 1;

    if(PilaVacia(pila))
    {
        printf("-");
        return;
    }

    CrearPila(&pilaAux);

    while(SacarDePila(pila, sizeof(tContenedor), &contenedor) != PILA_VACIA)
    {
        if(primero)
        {
            printf("%s <- Tope", contenedor.codigo);
            primero = 0;
        }
        else
            printf(" | %s", contenedor.codigo);
        printf("\n");
        PonerEnPila(&pilaAux, sizeof(tContenedor), &contenedor);
    }

    while(SacarDePila(&pilaAux, sizeof(tContenedor), &contenedor) != PILA_VACIA)
        PonerEnPila(pila, sizeof(tContenedor), &contenedor);
}

void ImprimirVentanaDePlanificacion(tCola *cola)
{
    tCola colaAux;
    tCamion *camion;
    int i = 0;

    if(ColaVacia(cola))
    {
        printf("No hay camiones esperando\n");
        return;
    }
    CrearCola(&colaAux);

    while(SacarDeLaCola(cola, sizeof(camion), &camion) != COLA_VACIA)
    {
        if(i < 3)
        {
            printf("%d) %s -> solicita %s", i + 1, camion->id, camion->pedido.codigo);

            if(i == 0)
                printf(" <- frente");

            printf("\n");
        }
        i++;
        PonerEnCola(&colaAux, sizeof(camion), &camion);
    }

    while(SacarDeLaCola(&colaAux, sizeof(camion), &camion) != COLA_VACIA)
        PonerEnCola(cola, sizeof(camion), &camion);
}
