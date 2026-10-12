#include "OpereacionesYComandos.h"

    /*--------------------------------------------------------------*/
    /* Compara el contenedor en el tope de la pila
        contenida en la la zona con codContenedor
    */
    /*--------------------------------------------------------------*/

int cmpContenedores(void* codContendor, void* zonaActual){
    tZona* zona= zonaActual;
    tPila* pilaCont;
    tContenedor contenedor;
    pilaCont=&(zona->pila);
    if(PilaVacia(pilaCont))
        return  CONTENEDOR_FALTANTE;
    VerTope(pilaCont,sizeof(tContenedor),&contenedor);

    return strcmp((char*)codContendor,contenedor.codigo);
}

    /*--------------------------------------------------------------*/
    /* Desapila el contenedor de la zona que llega como parametro
    */
    /*--------------------------------------------------------------*/
void liberarEspacioZona(void* zonaActual){
    tZona *zona= zonaActual;
    tPila* pilaCont;
    tContenedor contenedor;
    pilaCont=&(zona->pila);
    if(PilaVacia(pilaCont))
        return;
    SacarDePila(pilaCont,sizeof(tContenedor),&contenedor);
    zona->cantidad--;
}
int OperacionENT(tJornada* jornada){

    tCamion camionAtendido;
    tContenedor contBuscado;
    tCola* colaCamiones= &(jornada->puerto.colaCamiones);
    tLista* listaZonas=&(jornada->puerto.zonas);
    int resultadoBusqueda;
    int t_inicio = jornada->tiempoActual,
    t_fin = t_inicio + jornada->cfg.tiempoCargaCamion,
    t_finDeJornada= jornada->cfg.duracionJornadaMinutos;
    /*--------------------------------------------------------------*/
    /*1)Casos particulares de entrada
       1.1) Reviso que la cola de camiones no este vacia
       1.2)    "    "  la lista de zonas no este vacia
       1.3)    "    "  aun quede tiempo para realizar la operacion
    */
    /*--------------------------------------------------------------*/
    if(ColaVacia(colaCamiones))
        return COLA_CAMIONES_VACIA;
    if(ListaVacia(listaZonas))
        return LISTA_ZONAS_VACIA;
    if(t_fin>t_finDeJornada)
        return SIN_TIEMPO;
    /*--------------------------------------------------------------*/
    /* 2)Si ambas tienen al menos un elemento
        2.1)recupero de la cola de camiones el proximo camion
        2.2)guardo el contenedor del mismo para iniciar la busqueda
    */
    /*--------------------------------------------------------------*/
    VerPrimeroEnCola(colaCamiones,sizeof(tCamion),&camionAtendido);
    contBuscado=camionAtendido.pedido;
    /*--------------------------------------------------------------*/
    /* 3)Busco el contenedor dentro de la lista de zonas,
        modificando la cantidad de contenedores de la zona
        y desapilando si lo encuentra
        (Funcion usada en lista.h)
        (Comparacion y accion usada para la misma en esta misma hoja)
    */
    /*--------------------------------------------------------------*/
    resultadoBusqueda=buscarEnListaSinBorrar(listaZonas,
                                             contBuscado.codigo,
                                             NO_RECUPERAR,
                                             cmpContenedores,
                                             liberarEspacioZona);
    /*--------------------------------------------------------------*/
    /*  4)Si se encontro el contBuscado
            4.1) Saco al camion actual de la cola de camiones
            4.2) Avanzo el tiempoActual segun lo que tarde la operacion
    */
    /*--------------------------------------------------------------*/
    if(resultadoBusqueda){
        SacarDeLaCola(colaCamiones,sizeof(tCamion),&camionAtendido);
        jornada->tiempoActual= t_fin;
        return CONTENEDOR_ENTREGADO;
    }

    return CONTENEDOR_FALTANTE;
}

int cmpZona(void* infoLista, void* dato){
    tZona* zonaLista=infoLista,
         * zonaDato=dato;
    return zonaLista->id - zonaDato->id;


}
int cmpMuelle(void*infoLista, void*dato){
    tMuelle* muelleLista= infoLista,
           * muelleDato=dato;
    return muelleLista->id - muelleDato->id;

}
int operacionDES(tJornada* jornada, int muelle, int zonaDestino){

    tLista* muelles= &(jornada->puerto.muelles);
    tLista* zonas=&(jornada->puerto.zonas);
    tPila* contenedores;
    tMuelle* muelleActual={0};
    tZona* zonaActual={0};
    tContenedor contenedorObjetivo;
    int t_ini=jornada->tiempoActual,
    t_fin=t_ini + jornada->cfg.tiempoDescargaContenedor,
    t_finDeJornada= jornada->cfg.duracionJornadaMinutos,
    resultadoBusqueda,
    tamZonas=jornada->cfg.maximoContenedoresPorBuque;

    /*--------------------------------------------------------------*/
    /*1)Casos particulares de entrada
       1.1) Reviso que la lista de muelles no este vacia
       1.2)    "    "  la lista de zonas no este vacia
       1.3)    "    "  aun quede tiempo para realizar la operacion
    */
    /*--------------------------------------------------------------*/
    if(ListaVacia(muelles))
        return LISTA_MUELLES_VACIA;
    if(ListaVacia(zonas))
        return LISTA_ZONAS_VACIA;
    if(t_fin>t_finDeJornada)
        return SIN_TIEMPO;
    /*--------------------------------------------------------------*/
    /*2)Si ambas listas tienen datos
       2.1) Guardo el id del muelle y zona en muelleActual y zonaActual
       2.2) Uso el id guardado para realizar una busqueda en las listas
       2.3) Si no encuentro el muelle o la zona salgo de la funcion

    */
    /*--------------------------------------------------------------*/
    muelleActual->id=muelle;
    zonaActual->id=zonaDestino;
    resultadoBusqueda=buscarEnListaSinBorrar(muelles,
                                             muelleActual,
                                             sizeof(tMuelle),
                                             cmpMuelle,
                                             NULL);
    if(!resultadoBusqueda)
        return MUELLE_NO_VALIDO;
    resultadoBusqueda=buscarEnListaSinBorrar(zonas,
                                             zonaActual,
                                             sizeof(tZona),
                                             cmpZona,
                                             NULL);
    if(!resultadoBusqueda)
        return ZONA_NO_VALIDA;
   /*--------------------------------------------------------------*/
    /*3)Si hay espacio en zonaActual
       3.1) Saco el siguiente contenedor del buque
       3.2) Lo coloco en su zona correspondiente
    */
    /*--------------------------------------------------------------*/
    if(zonaActual->cantidad +1 > tamZonas)
        return ZONA_LLENA;

    contenedores=&(muelleActual->buque->manifiesto);
    SacarDePila(contenedores,sizeof(tContenedor),&contenedorObjetivo);
    contenedores=&(zonaActual->pila);
    PonerEnPila(contenedores,sizeof(tContenedor),&contenedorObjetivo);
    zonaActual->cantidad++;

    jornada->tiempoActual=t_fin;

    return MUELLE_DESCARGADO;
}
