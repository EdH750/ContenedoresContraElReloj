#include "ContenedoresContraReloj.h"
#include "jornada.h"
///#include "operador.h"

/* Punto de entrada del programa.
   El flujo principal es un do-while que limpia la pantalla, muestra el menu
   y ejecuta la opcion elegida. Al terminar cada opcion vuelve a limpiar y
   muestra el menu nuevamente, dando la sensacion de una pantalla dedicada
   por opcion.

   tConfiguracion y tJornada son locales a main: no son globales porque solo
   se usan en la opcion 1 y se pasan por puntero a cada funcion que las necesita. */
int main()
{
    int opcion;
    tConfiguracion configuracion;
    tJornada jornada;
    ///tOperador operador;

    /* srand se llama UNA SOLA VEZ al inicio del programa.
       Si se llamara dentro del loop, jornadas seguidas podrian generar
       los mismos numeros si las llamadas son muy rapidas (misma semilla) */
    srand((unsigned int)time(NULL));

    do {
        /* Limpiar la pantalla al inicio de cada iteracion para que el menu
           aparezca siempre desde cero, sin acumulacion de texto anterior */
        system("cls");
        menu();

        /* seleccionar_opcion lee una linea, la valida y carga el numero en opcion.
           Si la entrada es invalida deja OPCION_INVALIDA y el switch cae en default */
        seleccionar_opcion(&opcion);

        switch(opcion) {

            case 1:
                /* --- Opcion 1: Comenzar jornada ---
                   Limpiar el menu antes de arrancar para que la sesion de
                   identificacion y la jornada comiencen con pantalla limpia */
                system("cls");

                /* Paso 0: identificar al operario que va a jugar esta jornada.
                   Carga el nombre, busca en el archivo de operadores (con ABB),
                   registra al operario si es nuevo y llena la variable 'operador' */
                ///if(iniciar_sesion_operador(&operador) == 0) {
                ///    printf("Error al identificar al operador.\n");
                ///    printf("Presione Enter para volver al menu...");
                ///    getchar();
                ///    break;
                ///}

                /* Paso 1: leer y validar config.txt -> llena configuracion */
                if(procesar_configuracion(CONFIG_ARCHIVO, &configuracion) == 0) {
                    printf("Error al procesar el archivo de configuracion.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                /* Paso 2: generar puerto.txt con el escenario aleatorio */
                if(generar_archivo_puerto(&configuracion) == 0) {
                    printf("Error al generar el escenario de la jornada.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                /* Paso 3: leer puerto.txt, crear el puerto en vivo y dejar
                           la jornada en T=0. Si falla libera lo ya reservado */
                if(iniciar_jornada(&jornada, &configuracion) == 0) {
                    printf("Error al iniciar la jornada.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                /* Paso 4: ejecutar el bucle de la jornada. Al terminar
                   (fin normal, bloqueo o tiempo) muestra historial y resumen,
                   espera un Enter del operario y limpia la pantalla */
                if(ejecutar_jornada(&jornada) == 0) {
                    printf("La jornada se interrumpio por un error.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    /* Aun en caso de error intentamos guardar los resultados
                       para no perder los datos parciales */
                } else {
                    /* Paso 5: guardar los resultados en jornadas.bin.
                       Se hace ANTES de liberar porque necesita pJornada->resumen */
                    ///guardar_jornada(&jornada, &operador);
                }

                /* Paso 6: liberar toda la memoria reservada (manifiestos,
                   pilas de zonas, colas, historial). La pantalla ya esta
                   limpia gracias al system("cls") al final de ejecutar_jornada */
                liberar_jornada(&jornada);
                break;

            case 2:
                /* --- Opcion 2: Ver ranking ---
                   Mostrar el ranking de todos los operadores (leido de jornadas.bin) */
                system("cls");
                ///mostrar_ranking();
                printf("\nPresione Enter para volver al menu...");
                getchar();
                break;

            case 3:
                /* --- Opcion 3: Salir ---
                   Limpiar pantalla y despedirse antes de salir del loop */
                system("cls");
                printf("Saliendo del sistema...\n");
                break;

            default:
                /* Llega aca si seleccionar_opcion dejo OPCION_INVALIDA.
                   No se limpia la pantalla para que el error quede visible
                   junto al menu. Se pide Enter para que el usuario lo lea */
                printf("Opcion no valida, ingrese un numero entre %d y %d\n",
                       OPCION_MINIMA, OPCION_MAXIMA);
                printf("Presione Enter para continuar...");
                getchar();
                break;
        }

    } while(opcion != OPCION_SALIR);

    return 0;
}
