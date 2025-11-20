/**
 * @file destruirProcesoPar.c
 * @brief Implementación de la función para destruir un proceso par y liberar recursos
 */

#include "../include/ProcesoPar.h"
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>
#endif

/**
 * @brief Destruye un proceso par y libera todos los recursos
 */
Estado_t destruirProcesoPar(ProcesoPar_t *procesoPar) {
    /* Validar parámetro */
    if (procesoPar == NULL) {
        return E_PAR_INC;
    }

    /* Marcar el proceso como inactivo */
    procesoPar->activo = 0;

#ifdef _WIN32
    /* ========================================
     * IMPLEMENTACIÓN PARA WINDOWS
     * ======================================== */
    
    /* Saltar el cierre de tuberías que causa colgado en Windows */
    procesoPar->hTuberiaEntrada = NULL;
    procesoPar->hTuberiaSalida = NULL;

    /* Terminar el proceso hijo */
    if (procesoPar->hProceso != NULL) {
        TerminateProcess(procesoPar->hProceso, 0);
        CloseHandle(procesoPar->hProceso);
        procesoPar->hProceso = NULL;
    }

    /* Cerrar hilo de escucha */
    if (procesoPar->hHiloEscucha != NULL) {
        TerminateThread(procesoPar->hHiloEscucha, 0);
        CloseHandle(procesoPar->hHiloEscucha);
        procesoPar->hHiloEscucha = NULL;
    }

    if (procesoPar->hHilo != NULL) {
        CloseHandle(procesoPar->hHilo);
        procesoPar->hHilo = NULL;
    }

#else
    /* ========================================
     * IMPLEMENTACIÓN PARA LINUX
     * ======================================== */
    
    /* Cerrar tuberías */
    if (procesoPar->pipeEntrada[0] != -1) {
        close(procesoPar->pipeEntrada[0]);
        procesoPar->pipeEntrada[0] = -1;
    }

    if (procesoPar->pipeSalida[1] != -1) {
        close(procesoPar->pipeSalida[1]);
        procesoPar->pipeSalida[1] = -1;
    }

    /* Terminar el proceso hijo */
    if (procesoPar->pid > 0) {
        /* Verificar si el proceso existe primero */
        if (kill(procesoPar->pid, 0) == 0) {
            /* El proceso existe, intentar terminarlo */
            kill(procesoPar->pid, SIGTERM);
            
            /* Esperar brevemente que termine */
            int status;
            int result = waitpid(procesoPar->pid, &status, WNOHANG);
            
            /* Si no terminó inmediatamente, esperar un poco más */
            if (result == 0) {
                sleep(1);  /* Esperar 1 segundo */
                result = waitpid(procesoPar->pid, &status, WNOHANG);
                
                /* Si aún no terminó, forzar terminación */
                if (result == 0) {
                    kill(procesoPar->pid, SIGKILL);
                    waitpid(procesoPar->pid, &status, 0);
                }
            }
        }
        
        procesoPar->pid = -1;
    }

    /* Nota: El hilo de escucha terminará automáticamente cuando se cierren las tuberías
     * porque es "detached" y la lectura retornará 0 (EOF)
     */

#endif

    /* Liberar la memoria de la estructura */
    free(procesoPar);

    return E_OK;
}
