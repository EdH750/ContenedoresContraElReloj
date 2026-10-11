#include "ContenedoresContraReloj.h"
#include "jornada.h"

int main()
{

    int opcion;

    tConfiguracion configuracion;
    tJornada jornada;

    /* Semilla para numeros aleatorios, se llama una sola vez al inicio */
    srand((unsigned int)time(NULL));

    menu();

    do {

        seleccionar_opcion(&opcion);

        switch(opcion) {

            case 1:

                if(procesar_configuracion(CONFIG_ARCHIVO, &configuracion) == 0) {
                    printf("Error al procesar el archivo de configuracion\n");
                    break;
                }

                printf("Archivo de configuracion procesado correctamente\n");

                if(generar_archivo_puerto(&configuracion) == 0) {
                    printf("Error al generar el escenario de la jornada\n");
                    break;
                }

                if(iniciar_jornada(&jornada, &configuracion) == 0) {
                    printf("Error al iniciar la jornada\n");
                    break;
                }

                if(ejecutar_jornada(&jornada) == 0) {
                    printf("La jornada se interrumpio por un error\n");
                }

                liberar_jornada(&jornada);

                menu();

                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opcion no valida, ingrese un numero entre %d y %d\n", OPCION_MINIMA, OPCION_MAXIMA);
                break;

        }

    }while(opcion != OPCION_SALIR);

    return 0;
}
