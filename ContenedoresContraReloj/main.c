#include "menu.h"
#include "aplicacion.h"
#include "jornada.h"
#include "operador.h"

/*
Punto de entrada del programa.
Muestra el menu principal en un loop, ejecuta la opcion elegida y vuelve al inicio.
Devuelve 0 al sistema operativo al salir por la opcion 3 o por EOF.
*/
int main()
{
    int opcion;
    tConfiguracion configuracion;
    tJornada jornada;
    tOperador operador;

    srand((unsigned int)time(NULL));

    do {
        system("cls");
        menu();

        seleccionar_opcion(&opcion);

        switch(opcion) {

            case 1:
                system("cls");

                if(iniciar_sesion_operador(&operador) == ERROR) {
                    printf("Error al identificar al operador.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                if(procesar_configuracion(CONFIG_ARCHIVO, &configuracion) == ERROR) {
                    printf("Error al procesar el archivo de configuracion.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                if(generar_archivo_puerto(&configuracion) == ERROR) {
                    printf("Error al generar el escenario de la jornada.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                if(iniciar_jornada(&jornada, &configuracion) == ERROR) {
                    printf("Error al iniciar la jornada.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                    break;
                }

                if(ejecutar_jornada(&jornada) == ERROR) {
                    printf("La jornada se interrumpio por un error.\n");
                    printf("Presione Enter para volver al menu...");
                    getchar();
                } else {
                    guardar_jornada(&jornada, &operador);
                }

                liberar_jornada(&jornada);
                break;

            case 2:
                system("cls");
                mostrar_ranking();
                printf("\nPresione Enter para volver al menu...");
                getchar();
                break;

            case 3:
                system("cls");
                printf("Saliendo del sistema...\n");
                break;

            default:
                printf("Opcion no valida, ingrese un numero entre %d y %d\n",
                       OPCION_MINIMA, OPCION_MAXIMA);
                printf("Presione Enter para continuar...");
                getchar();
                break;
        }

    } while(opcion != OPCION_SALIR);

    return 0;
}
