#include "ContenedoresContraReloj.h"

void menu() {

    printf("============================PUERTO Y CONTENEDORES============================\n");
    printf("1)Comenzar jornada\n");
    printf("2)Ver ranking\n");
    printf("3)COMPLETAR\n");
    printf("4)Salir\n");

}

/* Si lo ingresado no es una opcion valida, deja OPCION_INVALIDA */
void seleccionar_opcion(int* opcion) {

    char buffer[MAX_BUFFER];
    int c;

    printf("Ingrese opcion: ");

    if(fgets(buffer, MAX_BUFFER, stdin) == NULL) {
        *opcion = OPCION_SALIR;
        return;
    }

    if(strchr(buffer, '\n') == NULL) {
        /* La linea no entro en el buffer: se descarta el resto y se rechaza */
        while((c = getchar()) != '\n' && c != EOF);
        buffer[0] = '\0';
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';

    if(validar_opcion(buffer, opcion) == 0) {
        *opcion = OPCION_INVALIDA;
    }
}

/* Solo digitos y entre OPCION_MINIMA y OPCION_MAXIMA: "", "f3", "3f", " 3", "+3" y "2.1" se rechazan */
int validar_opcion(const char* texto, int* opcion) {

    if(texto[0] == '\0') {
        return 0;
    }

    if(strspn(texto, "0123456789") != strlen(texto)) {
        return 0;
    }

    if(leer_entero(texto, opcion) == 0) {
        return 0;
    }

    return *opcion >= OPCION_MINIMA && *opcion <= OPCION_MAXIMA;
}

/* Nombres de los campos en el orden en que se leen de config.txt */
static const char* NOMBRES_CAMPOS[CANTIDAD_CAMPOS_CONFIG] = {
    "duracion_jornada_minutos",
    "cantidad_muelles",
    "cantidad_zonas_almacenamiento",
    "capacidad_pila",
    "maximo_buques",
    "maximo_contenedores_por_buque",
    "maximo_camiones",
    "tiempo_descarga_contenedor",
    "tiempo_reubicacion_contenedor",
    "tiempo_carga_camion"
};

int procesar_configuracion(const char* file_name, tConfiguracion* pConfiguracion) {

    FILE* pArchivo;
    int campo = 1;
    int ok = 1;
    int i;
    int leidos[CANTIDAD_CAMPOS_CONFIG] = {0};
    char buffer[MAX_BUFFER];

    pArchivo = fopen(file_name, "rt");

    if(pArchivo == NULL) {
        printf("Error al abrir el archivo configuracion\n");
        return 0;
    }

    while(ok && fgets(buffer, MAX_BUFFER, pArchivo) != NULL) {

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if(linea_vacia(buffer)) {
            continue;
        }

        if(campo > CANTIDAD_CAMPOS_CONFIG) {
            printf("Error en configuracion: hay mas de %d campos\n", CANTIDAD_CAMPOS_CONFIG);
            ok = 0;
        } else if(parsear_campos(buffer, pConfiguracion, campo) == 1) {
            leidos[campo - 1] = 1;
            campo++;
        } else {
            ok = 0;
        }

    }

    fclose(pArchivo);

    if(ok == 0) {
        return 0;
    }

    for(i = 0; i < CANTIDAD_CAMPOS_CONFIG; i++) {
        if(leidos[i] == 0) {
            printf("Error en configuracion: falta el campo %s\n", NOMBRES_CAMPOS[i]);
            ok = 0;
        }
    }

    if(ok == 0 || validar_campos(pConfiguracion) == 0) {
        printf("Los campos no cumplen los requisitos\n");
        return 0;
    }

    printf("Los campos cumplen los requisitos\n");

    return 1;
}

int linea_vacia(const char* linea) {

    while(*linea != '\0') {
        if(!isspace((unsigned char)*linea)) {
            return 0;
        }
        linea++;
    }

    return 1;
}

int parsear_campos(char* linea, tConfiguracion* pConfiguracion, int campo) {

    char* pLim;
    int valor;

    pLim = strrchr(linea, ':');

    if(pLim == NULL) {
        printf("Error en configuracion: falta ':' en el campo %s (\"%s\")\n", NOMBRES_CAMPOS[campo - 1], linea);
        return 0;
    }

    pLim++;

    if(leer_entero(pLim, &valor) == 0) {
        printf("Error en configuracion: %s no es un numero entero valido (\"%s\")\n", NOMBRES_CAMPOS[campo - 1], pLim);
        return 0;
    }

    switch(campo) {

        case 1:
            pConfiguracion->duracionJornadaMinutos = valor;
            break;
        case 2:
            pConfiguracion->cantidadMuelles = valor;
            break;
        case 3:
            pConfiguracion->cantidadZonasAlmacenamiento = valor;
            break;
        case 4:
            pConfiguracion->capacidadPila = valor;
            break;
        case 5:
            pConfiguracion->maximoBuques = valor;
            break;
        case 6:
            pConfiguracion->maximoContenedoresPorBuque = valor;
            break;
        case 7:
            pConfiguracion->maximoCamiones = valor;
            break;
        case 8:
            pConfiguracion->tiempoDescargaContenedor = valor;
            break;
        case 9:
            pConfiguracion->tiempoReubicacionContenedor = valor;
            break;
        case 10:
            pConfiguracion->tiempoCargaCamion = valor;
            break;

    }

    return 1;
}

int leer_entero(const char* texto, int* valor) {

    char* fin;
    long numero;

    errno = 0;
    numero = strtol(texto, &fin, 10);

    /* No habia ningun digito: "" o "abc" */
    if(fin == texto) {
        return 0;
    }

    while(isspace((unsigned char)*fin)) {
        fin++;
    }

    /* Sobran caracteres despues del numero: "5abc" o "2.7" */
    if(*fin != '\0') {
        return 0;
    }

    if(errno == ERANGE || numero < INT_MIN || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;

    return 1;
}

int validar_rango(int campo, int valor, int minimo, int maximo) {

    if(valor >= minimo && valor <= maximo) {
        return 1;
    }

    if(maximo == INT_MAX) {
        printf("Error en configuracion: %s debe ser mayor o igual a %d (se leyo %d)\n", NOMBRES_CAMPOS[campo - 1], minimo, valor);
    } else {
        printf("Error en configuracion: %s debe estar entre %d y %d (se leyo %d)\n", NOMBRES_CAMPOS[campo - 1], minimo, maximo, valor);
    }

    return 0;
}

int validar_campos(tConfiguracion* pConfiguracion) {

    int ok = 1;
    int duracion = pConfiguracion->duracionJornadaMinutos;

    /* Se evaluan todos los campos para informar todos los errores juntos */
    ok &= validar_rango(1,  pConfiguracion->duracionJornadaMinutos,      MINIMO_TIEMPO_JORNADA, MAXIMO_TIEMPO_JORNADA);
    ok &= validar_rango(2,  pConfiguracion->cantidadMuelles,             1, INT_MAX);
    ok &= validar_rango(3,  pConfiguracion->cantidadZonasAlmacenamiento, 1, INT_MAX);
    ok &= validar_rango(4,  pConfiguracion->capacidadPila,               1, INT_MAX);
    ok &= validar_rango(5,  pConfiguracion->maximoBuques,                1, MAXIMO_BUQUES);
    ok &= validar_rango(6,  pConfiguracion->maximoContenedoresPorBuque,  1, MAXIMO_CONTENEDORES_POR_BUQUE);
    ok &= validar_rango(7,  pConfiguracion->maximoCamiones,              1, MAXIMO_CAMIONES);
    ok &= validar_rango(8,  pConfiguracion->tiempoDescargaContenedor,    1, duracion);
    ok &= validar_rango(9,  pConfiguracion->tiempoReubicacionContenedor, 1, duracion);
    ok &= validar_rango(10, pConfiguracion->tiempoCargaCamion,           1, duracion);

    return ok;
}

int generar_archivo_puerto(tConfiguracion* pConfiguracion) {

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
        return 0;
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
        return 0;
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

    return 1;
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
