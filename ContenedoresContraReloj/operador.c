#include "operador.h"
#include "TDA/abb.h"

/* ─── Comparadores ──────────────────────────────────────────────────── */

/* Compara dos tOperador por nombre (para el ABB de sesion).
   Permite buscar un operador solo con su nombre (el campo id no importa) */
int cmp_operador_por_nombre(const void* a, const void* b) {
    const tOperador* oa = (const tOperador*)a;
    const tOperador* ob = (const tOperador*)b;
    return strcmp(oa->nombre, ob->nombre);
}

/* Compara una tEntradaRanking con un int* (el id del operador).
   Se usa con buscar_en_lista para encontrar la entrada acumulada de un operador */
int cmp_entrada_ranking_por_id(const void* dato, const void* clave) {
    return ((const tEntradaRanking*)dato)->idOperador - *(const int*)clave;
}

/* Criterio de orden para qsort al mostrar el ranking:
   1. Mayor puntuacion total (descendente)
   2. Menor jornadas jugadas (ascendente): llego al mismo puntaje en menos partidas
   3. Mayor entregas totales (descendente): como tercer desempate */
int cmp_ranking_qsort(const void* a, const void* b) {
    const tEntradaRanking* ra = (const tEntradaRanking*)a;
    const tEntradaRanking* rb = (const tEntradaRanking*)b;

    if(ra->totalPuntuacion != rb->totalPuntuacion)
        return rb->totalPuntuacion - ra->totalPuntuacion;

    if(ra->jornadasJugadas != rb->jornadasJugadas)
        return ra->jornadasJugadas - rb->jornadasJugadas;

    return rb->totalEntregados - ra->totalEntregados;
}

/* ─── Sesion de operador ────────────────────────────────────────────── */

/* Carga el archivo operadores.bin en el ABB (keyed por nombre).
   Cada nodo del ABB almacena un tOperador completo (id + nombre) */
static void cargar_abb_operadores(tABB* pa, const char* archivo) {
    FILE* pf;
    tOperador op;

    crear_abb(pa);

    /* Si el archivo no existe aun (primer operador), el ABB queda vacio */
    pf = fopen(archivo, "rb");
    if(!pf) return;

    while(fread(&op, sizeof(tOperador), 1, pf) == 1) {
        /* El ABB permite buscar en O(log n) en lugar de recorrer la lista */
        insertar_abb(pa, &op, sizeof(tOperador), cmp_operador_por_nombre);
    }
    fclose(pf);
}

/* Devuelve cuantos registros hay en el archivo (para asignar el proximo id) */
static int contar_registros_operadores(const char* archivo) {
    FILE* pf;
    int cant;

    pf = fopen(archivo, "rb");
    if(!pf) return 0;

    fseek(pf, 0, SEEK_END);
    cant = (int)(ftell(pf) / sizeof(tOperador));
    fclose(pf);
    return cant;
}

/* Guarda un nuevo operador al final de operadores.bin.
   "ab" crea el archivo si no existe y pone el puntero al final si existe */
static int persistir_operador(const tOperador* pOp) {
    FILE* pf = fopen(OPERADORES_ARCHIVO, "ab");
    if(!pf) return 0;
    fwrite(pOp, sizeof(tOperador), 1, pf);
    fclose(pf);
    return 1;
}

/* Lee una linea del teclado y la deja en nombre (sin '\n', max MAX_NOMBRE_OPERADOR).
   Si la linea no entraba en el buffer, descarta el resto para no contaminar.
   Devuelve 1 si leyo algo no vacio, 0 si el nombre quedo vacio */
static int leer_nombre(char* nombre) {
    int c;

    printf("Nombre del operador: ");
    fgets(nombre, MAX_NOMBRE_OPERADOR, stdin);

    /* Si el '\n' no entro en el buffer, hay caracteres extra pendientes */
    if(strchr(nombre, '\n') == NULL) {
        while((c = getchar()) != '\n' && c != EOF);
    }

    nombre[strcspn(nombre, "\r\n")] = '\0';

    return nombre[0] != '\0';
}

/* Pide el nombre al operario, lo busca en operadores.bin via ABB y llena *pOperador.
   Si no existe lo registra como nuevo asignandole el proximo id disponible.
   Retorna 1 si todo salio bien, 0 en caso de error (nombre vacio o malloc fallo) */
int iniciar_sesion_operador(tOperador* pOperador) {

    tABB indice;
    tOperador busqueda;
    tOperador* encontrado;
    char nombre[MAX_NOMBRE_OPERADOR];

    /* 1. Cargar el ABB con los operadores ya registrados */
    cargar_abb_operadores(&indice, OPERADORES_ARCHIVO);

    /* 2. Pedir nombre (con reintento si queda vacio) */
    do {
        if(!leer_nombre(nombre)) {
            printf("El nombre no puede estar vacio. Ingrese nuevamente.\n");
        }
    } while(nombre[0] == '\0');

    /* 3. Buscar en el ABB: cmp_operador_por_nombre solo usa el campo nombre,
          por lo que el id del nodo de busqueda no importa */
    strncpy(busqueda.nombre, nombre, MAX_NOMBRE_OPERADOR - 1);
    busqueda.nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';
    busqueda.id = 0;   /* ignorado por el comparador */

    encontrado = (tOperador*)buscar_abb(&indice, &busqueda, cmp_operador_por_nombre);

    if(encontrado) {
        /* Operador conocido: cargar su id y nombre */
        *pOperador = *encontrado;
        printf("Bienvenido de vuelta, %s! (ID: %d)\n", pOperador->nombre, pOperador->id);

    } else {
        /* Nuevo operador: asignar id secuencial y persistir en el archivo */
        pOperador->id = contar_registros_operadores(OPERADORES_ARCHIVO) + 1;
        strncpy(pOperador->nombre, nombre, MAX_NOMBRE_OPERADOR - 1);
        pOperador->nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';

        if(!persistir_operador(pOperador)) {
            printf("Error: no se pudo registrar el operador en el archivo.\n");
            vaciar_abb(&indice);
            return 0;
        }
        printf("Nuevo operador registrado: %s (ID: %d)\n", pOperador->nombre, pOperador->id);
    }

    /* 4. Liberar el ABB (solo sirvio para esta sesion) */
    vaciar_abb(&indice);
    return 1;
}

/* ─── Guardado de jornada ────────────────────────────────────────────── */

/* Escribe los resultados de la jornada en jornadas.bin.
   Se llama ANTES de liberar_jornada porque necesita pJornada->resumen.
   El idJornada se calcula como la cantidad de registros ya existentes + 1 */
int guardar_jornada(const tJornada* pJornada, const tOperador* pOperador) {

    FILE* pf;
    tRegistroJornada registro;
    long cantReg;

    /* "a+b": crea si no existe, agrega al final si existe */
    pf = fopen(JORNADAS_ARCHIVO, "a+b");
    if(!pf) {
        printf("Advertencia: no se pudo guardar la jornada en %s\n", JORNADAS_ARCHIVO);
        return 0;
    }

    /* Calcular el id secuencial */
    fseek(pf, 0, SEEK_END);
    cantReg = ftell(pf) / sizeof(tRegistroJornada);

    /* Llenar el registro con los datos de la jornada */
    registro.idJornada               = (int)cantReg + 1;
    registro.idOperador              = pOperador->id;
    strncpy(registro.nombreOperador, pOperador->nombre, MAX_NOMBRE_OPERADOR - 1);
    registro.nombreOperador[MAX_NOMBRE_OPERADOR - 1] = '\0';
    registro.puntuacion              = pJornada->resumen.puntuacion;
    registro.contenedoresEntregados  = pJornada->resumen.contenedoresEntregados;
    registro.buquesDescargados       = pJornada->resumen.buquesDescargados;
    registro.camionesPendientes      = pJornada->resumen.camionesPendientes;
    registro.reubicaciones           = pJornada->resumen.reubicaciones;
    registro.tiempoUtilizado         = pJornada->resumen.tiempoUtilizado;
    registro.motivoFin               = (int)pJornada->resumen.motivoFin;

    fwrite(&registro, sizeof(tRegistroJornada), 1, pf);
    fclose(pf);
    return 1;
}

/* ─── Ranking ────────────────────────────────────────────────────────── */

/* Lee jornadas.bin, acumula resultados por operador usando una lista,
   extrae a un array, ordena con qsort y muestra el ranking */
void mostrar_ranking(void) {

    FILE* pf;
    tRegistroJornada reg;
    tLista lista;
    tEntradaRanking* pEnt;
    tEntradaRanking nueva;
    tEntradaRanking entrada;
    tEntradaRanking* array;
    tEntradaRanking* temp;
    int n;
    int i;

    pf = fopen(JORNADAS_ARCHIVO, "rb");
    if(!pf) {
        printf("  Aun no hay jornadas registradas.\n");
        return;
    }

    crear_lista(&lista);
    n = 0;

    /* Leer todos los registros y acumular por operador */
    while(fread(&reg, sizeof(tRegistroJornada), 1, pf) == 1) {

        /* buscar_en_lista devuelve un puntero al dato del nodo (no una copia),
           por lo que modificarlo cambia directamente el elemento en la lista */
        pEnt = (tEntradaRanking*)buscar_en_lista(&lista,
                                                   &reg.idOperador,
                                                   cmp_entrada_ranking_por_id);
        if(pEnt) {
            /* Operador ya en la lista: acumular resultados */
            pEnt->totalPuntuacion       += reg.puntuacion;
            pEnt->jornadasJugadas++;
            pEnt->totalEntregados       += reg.contenedoresEntregados;
            pEnt->totalBuquesDescargados += reg.buquesDescargados;
        } else {
            /* Nuevo operador: agregar entrada inicial */
            nueva.idOperador              = reg.idOperador;
            strncpy(nueva.nombre, reg.nombreOperador, MAX_NOMBRE_OPERADOR - 1);
            nueva.nombre[MAX_NOMBRE_OPERADOR - 1] = '\0';
            nueva.totalPuntuacion         = reg.puntuacion;
            nueva.jornadasJugadas         = 1;
            nueva.totalEntregados         = reg.contenedoresEntregados;
            nueva.totalBuquesDescargados  = reg.buquesDescargados;

            if(enlistar_final(&lista, &nueva, sizeof(tEntradaRanking)) == 1) {
                n++;
            }
        }
    }
    fclose(pf);

    if(n == 0) {
        printf("  No se pudo leer el historial de jornadas.\n");
        vaciar_lista(&lista);
        return;
    }

    /* Extraer la lista a un array dinamico para poder usar qsort.
       Se usa realloc de a un elemento porque n es pequeno (decenas de operadores) */
    array = NULL;
    i     = 0;
    while(desenlistar_inicio(&lista, &entrada, sizeof(tEntradaRanking)) == 1) {
        temp = (tEntradaRanking*)realloc(array, (i + 1) * sizeof(tEntradaRanking));
        if(!temp) {
            free(array);
            printf("  Error de memoria al generar el ranking.\n");
            return;
        }
        array = temp;
        array[i++] = entrada;
    }

    /* Ordenar el array con qsort segun el criterio del ranking */
    qsort(array, n, sizeof(tEntradaRanking), cmp_ranking_qsort);

    /* Mostrar tabla */
    printf("\n");
    printf("#==========================================================#\n");
    printf("  RANKING DE OPERADORES\n");
    printf("#==========================================================#\n");
    printf("  %-4s %-22s %10s %9s %9s\n",
           "Pos", "Operador", "Puntuacion", "Jornadas", "Entregas");
    printf("  ----+----------------------+----------+---------+---------\n");

    for(i = 0; i < n; i++) {
        printf("  %-4d %-22s %10d %9d %9d\n",
               i + 1,
               array[i].nombre,
               array[i].totalPuntuacion,
               array[i].jornadasJugadas,
               array[i].totalEntregados);
    }

    printf("#==========================================================#\n");
    printf("  Criterios: 1) Mayor puntuacion total\n");
    printf("             2) Menor jornadas jugadas (desempate)\n");
    printf("             3) Mayor entregas totales (desempate)\n");
    printf("#==========================================================#\n");

    free(array);
}
