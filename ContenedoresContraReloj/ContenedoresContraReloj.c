#include "ContenedoresContraReloj.h"

void menu() {

    printf("============================PUERTO Y CONTENEDORES============================\n");
    printf("1)Comenzar jornada\n");
    printf("2)Ver ranking\n");
    printf("3)COMPLETAR\n");
    printf("4)Salir\n");

}

void seleccionar_opcion(int* opcion) {

    printf("Ingrese opcion: ");
    scanf("%d", opcion);

}

int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion) {

    FILE* pArchivo;
    int campo = 1;
    char buffer[MAX_BUFFER];

    pArchivo = fopen(file_name, "rt");

    if(pArchivo == NULL) {
        printf("Error al abrir el archivo configuracion\n");
        return 0;
    }

    while(fgets(buffer, MAX_BUFFER, pArchivo) != NULL) {

        parsear_campos(buffer, pConfiguracion, campo);
        campo++;

    }

    if(validar_campos(pConfiguracion) == 0) {
        printf("Los campos no cumplen los requisitos\n");
        return 0;
    }

    printf("Los campos cumplen los requisitos\n");

    fclose(pArchivo);

    return 1;
}

void parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo) {

    char* pLim;

    pLim = strrchr(linea, ':');

    if(pLim == NULL) {
        return;
    }

    pLim++;

    switch(campo) {

        case 1:
            sscanf(pLim, "%d", &pConfiguracion->duracionJornadaMinutos);
            break;
        case 2:
            sscanf(pLim, "%d", &pConfiguracion->cantidadMuelles);
            break;
        case 3:
            sscanf(pLim, "%d", &pConfiguracion->cantidadZonasAlmacenamiento);
            break;
        case 4:
            sscanf(pLim, "%d", &pConfiguracion->capacidadPila);
            break;
        case 5:
            sscanf(pLim, "%d", &pConfiguracion->maximoBuques);
            break;
        case 6:
            sscanf(pLim, "%d", &pConfiguracion->maximoContenedoresPorBuque);
            break;
        case 7:
            sscanf(pLim, "%d", &pConfiguracion->maximoCamiones);
            break;
        case 8:
            sscanf(pLim, "%d", &pConfiguracion->tiempoDescargaContenedor);
            break;
        case 9:
            sscanf(pLim, "%d", &pConfiguracion->tiempoReubicacionContenedor);
            break;
        case 10:
            sscanf(pLim, "%d", &pConfiguracion->tiempoCargaCamion);
            break;
        default:
            printf("Campo no valido en archivo configuracion.txt\n");
            break;

    }

}

int validar_campos(tConfiguracion* pConfiguracion) {

    if(pConfiguracion->duracionJornadaMinutos < MINIMO_TIEMPO_JORNADA || pConfiguracion->duracionJornadaMinutos > MAXIMO_TIEMPO_JORNADA) {
        return 0;
    }

    if(pConfiguracion->tiempoCargaCamion > pConfiguracion->duracionJornadaMinutos || pConfiguracion->tiempoDescargaContenedor > pConfiguracion->duracionJornadaMinutos || pConfiguracion->tiempoReubicacionContenedor > pConfiguracion->duracionJornadaMinutos) {
        return 0;
    }

    return 1;
}

void generar_archivo_puerto(tConfiguracion* pConfiguracion) {

    /* --- Todas las variables declaradas al inicio del bloque --- */
    FILE* pArchivo;
    int i, j, k;
    int maxContenedoresTotal;
    int numBuques;
    int totalContenedores;
    int maxCamionesValido;
    int numCamiones;
    char tmp[MAX_COD_CONTENEDOR];

    int*              buqueT;       /* tiempo de llegada de cada buque      */
    int*              buqueNumCont; /* cantidad de contenedores de cada buque */
    int*              camionT;      /* tiempo de llegada de cada camion     */
    char (*pool)[MAX_COD_CONTENEDOR]; /* pool global de codigos de contenedores */

    /* ------------------------------------------------------------------ */
    /* Calculo del tope maximo posible de contenedores usando el struct    */
    /* ------------------------------------------------------------------ */
    maxContenedoresTotal = pConfiguracion->maximoBuques * pConfiguracion->maximoContenedoresPorBuque;

    /* ------------------------------------------------------------------ */
    /* Reserva de memoria dinamica, sin VLAs ni macros hardcodeadas        */
    /* ------------------------------------------------------------------ */
    buqueT       = (int*)malloc(pConfiguracion->maximoBuques * sizeof(int));
    buqueNumCont = (int*)malloc(pConfiguracion->maximoBuques * sizeof(int));
    camionT      = (int*)malloc(pConfiguracion->maximoCamiones * sizeof(int));

    /*  pool es un puntero a arrays de MAX_COD_CONTENEDOR chars.
        Es equivalente a: char pool[maxContenedoresTotal][MAX_COD_CONTENEDOR]
        pero en el heap.                                                    */
    pool = (char(*)[MAX_COD_CONTENEDOR])malloc(maxContenedoresTotal * sizeof(*pool));

    if(buqueT == NULL || buqueNumCont == NULL || camionT == NULL || pool == NULL) {
        printf("Error al reservar memoria\n");
        free(buqueT);
        free(buqueNumCont);
        free(camionT);
        free(pool);
        return;
    }

    /* ------------------------------------------------------------------ */
    /* Generacion de buques y sus contenedores                             */
    /* ------------------------------------------------------------------ */
    numBuques        = (rand() % pConfiguracion->maximoBuques) + 1;
    totalContenedores = 0;

    /*  Los tiempos se sortean y ordenan antes de numerar los buques, asi
        B001 es siempre el primero en llegar y puerto.txt queda ordenado
        por T.                                                          */
    for(i = 0; i < numBuques; i++) {
        buqueT[i] = rand() % pConfiguracion->duracionJornadaMinutos;
    }
    ordenar_tiempos(buqueT, numBuques);

    for(i = 0; i < numBuques; i++) {

        buqueNumCont[i] = (rand() % pConfiguracion->maximoContenedoresPorBuque) + 1;

        for(j = 0; j < buqueNumCont[i]; j++) {
            /*  Escribe el codigo en la fila [totalContenedores] del pool.
                pool[totalContenedores] es un char[] de MAX_COD_CONTENEDOR.
                "C%d%02d": 'C' + nro buque (sin padding) + nro contenedor (2 digitos).
                Ej: buque 1, contenedor 3 -> "C103"
                    buque 2, contenedor 1 -> "C201"                         */
            sprintf(pool[totalContenedores], "C%d%02d", i + 1, j + 1);
            totalContenedores++;
        }
    }

    /* ------------------------------------------------------------------ */
    /* Shuffle Fisher-Yates sobre el pool para asignar contenedores        */
    /* al azar a los camiones sin repeticion                               */
    /* ------------------------------------------------------------------ */
    for(i = totalContenedores - 1; i > 0; i--) {
        j = rand() % (i + 1);
        strcpy(tmp, pool[i]);
        strcpy(pool[i], pool[j]);
        strcpy(pool[j], tmp);
    }

    /* ------------------------------------------------------------------ */
    /* Cantidad de camiones: entre 1 y min(maximoCamiones, contenedores)  */
    /* Un camion no puede pedir un contenedor que no existe               */
    /* ------------------------------------------------------------------ */
    maxCamionesValido = pConfiguracion->maximoCamiones;
    if(totalContenedores < maxCamionesValido) {
        maxCamionesValido = totalContenedores;
    }
    numCamiones = (rand() % maxCamionesValido) + 1;

    for(k = 0; k < numCamiones; k++) {
        camionT[k] = rand() % pConfiguracion->duracionJornadaMinutos;
    }
    ordenar_tiempos(camionT, numCamiones);

    /* ------------------------------------------------------------------ */
    /* Escritura del archivo puerto.txt                                    */
    /* ------------------------------------------------------------------ */
    pArchivo = fopen(PUERTO_ARCHIVO, "wt");
    if(pArchivo == NULL) {
        printf("Error al crear el archivo %s\n", PUERTO_ARCHIVO);
        free(buqueT);
        free(buqueNumCont);
        free(camionT);
        free(pool);
        return;
    }

    fprintf(pArchivo, "JORNADA:%d\n",        pConfiguracion->duracionJornadaMinutos);
    fprintf(pArchivo, "MUELLES:%d\n",        pConfiguracion->cantidadMuelles);
    fprintf(pArchivo, "ZONAS:%d\n",          pConfiguracion->cantidadZonasAlmacenamiento);
    fprintf(pArchivo, "CAPACIDAD_PILA:%d\n", pConfiguracion->capacidadPila);
    fprintf(pArchivo, "\n");

    fprintf(pArchivo, "[BUQUES]\n");
    for(i = 0; i < numBuques; i++) {
        fprintf(pArchivo, "B%03d;T=%d;C=", i + 1, buqueT[i]);
        for(j = 0; j < buqueNumCont[i]; j++) {
            if(j > 0) fprintf(pArchivo, ",");
            fprintf(pArchivo, "C%d%02d", i + 1, j + 1);
        }
        fprintf(pArchivo, "\n");
    }

    fprintf(pArchivo, "\n");

    fprintf(pArchivo, "[CAMIONES]\n");
    for(k = 0; k < numCamiones; k++) {
        fprintf(pArchivo, "K%03d;T=%d;C=%s\n", k + 1, camionT[k], pool[k]);
    }

    fclose(pArchivo);

    printf("Archivo %s generado: %d buque(s), %d camion(es), %d contenedor(es) en pool.\n",
           PUERTO_ARCHIVO, numBuques, numCamiones, totalContenedores);

    /* ------------------------------------------------------------------ */
    /* Liberacion de memoria                                               */
    /* ------------------------------------------------------------------ */
    free(buqueT);
    free(buqueNumCont);
    free(camionT);
    free(pool);
}

void ordenar_tiempos(int* tiempos, int cantidad) {

    int i, j;
    int actual;

    /* Insercion: es estable, los tiempos iguales conservan su orden */
    for(i = 1; i < cantidad; i++) {

        actual = tiempos[i];
        j = i - 1;

        while(j >= 0 && tiempos[j] > actual) {
            tiempos[j + 1] = tiempos[j];
            j--;
        }

        tiempos[j + 1] = actual;
    }

}



























