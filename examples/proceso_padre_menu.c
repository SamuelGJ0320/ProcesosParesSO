#include "../include/ProcesoPar.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/**
 * Función de escucha para mensajes del hijo.
 * Se llamará cada vez que el proceso hijo envíe algo al padre.
 */
static Estado_t funcionEscucha(const char *mensaje, int longitud) {
    printf("\n[PADRE] Mensaje desde el hijo (%d bytes): \"%.*s\"\n",
           longitud, longitud, mensaje);
    return E_OK;
}

int main(void) {
    int opcion = 0;
    Estado_t estado;
    ProcesoPar_t *proceso = NULL;   /* será rellenado por lanzarProcesoPar */

    printf("===== MENÚ PROCESO PADRE =====\n");

    /* Lanzar proceso hijo: ruta y argv según plataforma */
#ifdef _WIN32
    {
        const char *lineaComandoWin[] = {
            "proceso_hijo.exe",
            NULL
        };
        estado = lanzarProcesoPar("proceso_hijo.exe", (char * const *)lineaComandoWin, &proceso);
    }
#else
    {
        const char *lineaComandoPos[] = {
            "./proceso_hijo",
            NULL
        };
        estado = lanzarProcesoPar("./proceso_hijo", lineaComandoPos, &proceso);
    }
#endif

    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] No se pudo lanzar el proceso hijo. Código: %u errno=%d (%s)\n",
                (unsigned)estado, errno, strerror(errno));
        return EXIT_FAILURE;
    }

    /* DEBUG: mostrar pid y fds en la estructura (solo POSIX muestra fds) */
#ifndef _WIN32
    fprintf(stderr, "[DEBUG][padre_menu] hijo pid=%d pipeEntrada[0]=%d pipeSalida[1]=%d\n",
            (int)proceso->pid,
            proceso->pipeEntrada[0], proceso->pipeSalida[1]);
#else
    fprintf(stderr, "[DEBUG][padre_menu] hijo pid=%d\n", (int)proceso->pid);
#endif

    /* Establecer función de escucha para respuestas del hijo */
    estado = establecerFuncionDeEscucha(proceso, funcionEscucha);
    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] No se pudo establecer la función de escucha. Código: %u\n", (unsigned)estado);
        destruirProcesoPar(proceso);
        return EXIT_FAILURE;
    }

    /* Bucle de interacción */
    do {
        printf("\n¿Qué deseas hacer?\n");
        printf("1. Enviar mensaje al proceso hijo\n");
        printf("2. Salir\n");
        printf("Opción: ");

        if (scanf("%d", &opcion) != 1) {
            fprintf(stderr, "[ERROR] Entrada inválida\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            opcion = 0;
            continue;
        }

        if (opcion == 1) {
            char mensaje[512];

            /* limpiar '\n' que dejó scanf */
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}

            printf("Escribe tu mensaje (ej. HOLA, PING, SALIR): ");

            if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
                fprintf(stderr, "[ERROR] No se pudo leer el mensaje.\n");
                continue;
            }

            /* Asegurar que el mensaje termine con '\n' para que el hijo lo lea con fgets */
            size_t len = strlen(mensaje);
            if (len == 0) {
                fprintf(stderr, "[ERROR] Mensaje vacío.\n");
                continue;
            }

            /* enviar mensaje (longitud sin incluir terminador NUL) */
            estado = enviarMensajeProcesoPar(proceso, mensaje, (int)len);
            if (estado != E_OK) {
                fprintf(stderr, "[ERROR] No se pudo enviar el mensaje. Código: %u\n", (unsigned)estado);
            } else {
                printf("[INFO] Mensaje enviado correctamente.\n");
            }
        }

    } while (opcion != 2);

    /* Destruir proceso par */
    estado = destruirProcesoPar(proceso);
    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] Error al destruir el proceso par. Código: %u\n", (unsigned)estado);
    }

    printf("\nProceso padre finalizado.\n");
    return EXIT_SUCCESS;
}