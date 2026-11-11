#include "escenario.h"

/*
Lee puerto.txt y construye las listas de buques y camiones del escenario.
Devuelve EXITO si todo salio bien, ERROR si hubo un problema de memoria o lectura.
*/
int cargar_escenario(const char* nombreArchivo, const tConfiguracion* pConfiguracion, tEscenario* pEscenario) {

    FILE* pArchivo;
    char* linea;
    int tamLinea;
    int ok = EXITO;
    tSeccion seccion = SECCION_CABECERA;
    tBuque buque;
    tCamion camion;

    crear_lista(&pEscenario->buques);
    crear_lista(&pEscenario->camiones);

    tamLinea = LARGO_FIJO_LINEA + pConfiguracion->maximoContenedoresPorBuque * MAX_COD_CONTENEDOR;
    linea = (char*)malloc(tamLinea);

    if(linea == NULL) {
        printf("Error al reservar memoria para el escenario\n");
        return ERROR;
    }

    pArchivo = fopen(nombreArchivo, "rt");

    if(pArchivo == NULL) {
        printf("Error al abrir el archivo %s\n", nombreArchivo);
        free(linea);
        return ERROR;
    }

    while(ok && fgets(linea, tamLinea, pArchivo) != NULL) {

        linea[strcspn(linea, "\r\n")] = '\0';

        if(linea[0] == '\0') {
            continue;
        }

        if(strcmp(linea, "[BUQUES]") == 0) {
            seccion = SECCION_BUQUES;
        } else if(strcmp(linea, "[CAMIONES]") == 0) {
            seccion = SECCION_CAMIONES;
        } else if(seccion == SECCION_BUQUES) {
            if(cargar_buque(linea, &buque) == ERROR) {
                ok = ERROR;
            } else {
                enlistar_final(&pEscenario->buques, &buque, sizeof(tBuque));
            }
        } else if(seccion == SECCION_CAMIONES) {
            cargar_camion(linea, &camion);
            enlistar_final(&pEscenario->camiones, &camion, sizeof(tCamion));
        }
    }

    fclose(pArchivo);
    free(linea);

    if(ok == ERROR) {
        printf("Error al reservar memoria para el escenario\n");
        liberar_escenario(pEscenario);
        return ERROR;
    }

    return EXITO;
}

/*
Libera los buques y camiones del escenario que todavia no llegaron al puerto.
*/
void liberar_escenario(tEscenario* pEscenario) {

    tBuque buque;

    while(!lista_vacia(&pEscenario->buques)) {
        desenlistar_inicio(&pEscenario->buques, &buque, sizeof(tBuque));
        free(buque.manifiesto);
    }

    vaciar_lista(&pEscenario->camiones);
}

/*
Parsea una linea de buque con formato "B001;T=12;C=C101,C102" y llena pBuque.
Reserva memoria para el manifiesto.
Devuelve EXITO si pudo alojar el manifiesto, ERROR en caso contrario.
*/
int cargar_buque(char* linea, tBuque* pBuque) {

    char* pTiempo;
    char* pCodigos;
    char* pComa;
    int i;

    pTiempo = strchr(linea, ';');
    *pTiempo = '\0';
    pTiempo++;

    pCodigos = strchr(pTiempo, ';');
    *pCodigos = '\0';
    pCodigos += LARGO_PREFIJO_CODIGOS;

    strcpy(pBuque->id, linea);
    sscanf(pTiempo, "T=%d", &pBuque->tiempoLlegada);

    pBuque->cantContenedores  = contar_codigos(pCodigos);
    pBuque->proximoADescargar = 0;
    pBuque->manifiesto        = (tContenedor*)malloc(pBuque->cantContenedores * sizeof(tContenedor));

    if(pBuque->manifiesto == NULL) {
        return ERROR;
    }

    for(i = 0; i < pBuque->cantContenedores; i++) {

        pComa = strchr(pCodigos, ',');

        if(pComa != NULL) {
            *pComa = '\0';
        }

        strcpy(pBuque->manifiesto[i].codigo, pCodigos);

        if(pComa != NULL) {
            pCodigos = pComa + 1;
        }
    }

    return EXITO;
}

/*
Parsea una linea de camion con formato "K001;T=18;C=C102" y llena pCamion.
*/
void cargar_camion(char* linea, tCamion* pCamion) {

    char* pTiempo;
    char* pCodigo;

    pTiempo = strchr(linea, ';');
    *pTiempo = '\0';
    pTiempo++;

    pCodigo = strchr(pTiempo, ';');
    *pCodigo = '\0';
    pCodigo += LARGO_PREFIJO_CODIGOS;

    strcpy(pCamion->id, linea);
    sscanf(pTiempo, "T=%d", &pCamion->tiempoLlegada);
    strcpy(pCamion->pedido.codigo, pCodigo);
}

/*
Cuenta la cantidad de codigos separados por coma en la cadena.
Ejemplo: "C101,C102,C103" devuelve 3.
*/
int contar_codigos(const char* codigos) {

    int cantidad = 1;

    while(*codigos != '\0') {
        if(*codigos == ',') {
            cantidad++;
        }
        codigos++;
    }

    return cantidad;
}
