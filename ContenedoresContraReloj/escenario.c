#include "escenario.h"

/* puerto.txt lo escribe generar_archivo_puerto con una configuracion ya
   validada, por eso no se valida su contenido: si cambia el formato del
   generador, hay que cambiar tambien esta lectura. */

int cargar_escenario(const char* nombreArchivo, const tConfiguracion* pConfiguracion, tEscenario* pEscenario) {

    FILE* pArchivo;
    char* linea;
    int tamLinea;
    int ok = 1;
    tSeccion seccion = SECCION_CABECERA;
    tBuque buque;
    tCamion camion;

    crear_lista(&pEscenario->buques);
    crear_lista(&pEscenario->camiones);

    /* La linea mas larga es la de un buque con el maximo de contenedores */
    tamLinea = LARGO_FIJO_LINEA + pConfiguracion->maximoContenedoresPorBuque * MAX_COD_CONTENEDOR;

    linea = (char*)malloc(tamLinea);

    if(linea == NULL) {
        printf("Error al reservar memoria para el escenario\n");
        return 0;
    }

    pArchivo = fopen(nombreArchivo, "rt");

    if(pArchivo == NULL) {
        printf("Error al abrir el archivo %s\n", nombreArchivo);
        free(linea);
        return 0;
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
            if(cargar_buque(linea, &buque) == 0) {
                ok = 0;
            } else if(enlistar_final(&pEscenario->buques, &buque, sizeof(tBuque)) == 0) {
                /* El buque no llego a la lista: su manifiesto no lo libera liberar_escenario */
                free(buque.manifiesto);
                ok = 0;
            }
        } else if(seccion == SECCION_CAMIONES) {
            cargar_camion(linea, &camion);
            if(enlistar_final(&pEscenario->camiones, &camion, sizeof(tCamion)) == 0) {
                ok = 0;
            }
        }
        /* La cabecera se ignora: sus valores ya estan en la configuracion */

    }

    fclose(pArchivo);
    free(linea);

    if(ok == 0) {
        printf("Error al reservar memoria para el escenario\n");
        liberar_escenario(pEscenario);
        return 0;
    }

    return 1;
}

/* Libera los buques y camiones que todavia no llegaron */
void liberar_escenario(tEscenario* pEscenario) {

    tBuque buque;

    /* vaciar_lista no libera el manifiesto que esta dentro de cada tBuque */
    while(desenlistar_inicio(&pEscenario->buques, &buque, sizeof(tBuque)) == 1) {
        free(buque.manifiesto);
    }

    vaciar_lista(&pEscenario->camiones);
}

/* linea: "B001;T=12;C=C101,C102" */
int cargar_buque(char* linea, tBuque* pBuque) {

    char* pTiempo;
    char* pCodigos;
    char* pComa;
    int i;

    pTiempo = strchr(linea, ';');
    *pTiempo = '\0';
    pTiempo++;                                   /* "T=12;C=C101,C102" */

    pCodigos = strchr(pTiempo, ';');
    *pCodigos = '\0';
    pCodigos += 3;                               /* saltea ";C=" -> "C101,C102" */

    strcpy(pBuque->id, linea);
    sscanf(pTiempo, "T=%d", &pBuque->tiempoLlegada);

    pBuque->cantContenedores  = contar_codigos(pCodigos);
    pBuque->proximoADescargar = 0;
    pBuque->manifiesto        = (tContenedor*)malloc(pBuque->cantContenedores * sizeof(tContenedor));

    if(pBuque->manifiesto == NULL) {
        return 0;
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

    return 1;
}

/* linea: "K001;T=18;C=C102" */
void cargar_camion(char* linea, tCamion* pCamion) {

    char* pTiempo;
    char* pCodigo;

    pTiempo = strchr(linea, ';');
    *pTiempo = '\0';
    pTiempo++;                                   /* "T=18;C=C102" */

    pCodigo = strchr(pTiempo, ';');
    *pCodigo = '\0';
    pCodigo += 3;                                /* saltea ";C=" -> "C102" */

    strcpy(pCamion->id, linea);
    sscanf(pTiempo, "T=%d", &pCamion->tiempoLlegada);
    strcpy(pCamion->pedido.codigo, pCodigo);
}

/* "C101,C102,C103" -> 3 */
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
