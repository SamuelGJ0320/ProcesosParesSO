/**
* @file 
* @brief
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
 * @brief
 */
Estado_t destruirProcesoPar(ProcesoPar_t *procesoPar) {
    if (procesoPar == NULL) {
        return E_PAR_INC;
    }

    /* Marcar el proceso como inactivo */
    procesoPar->activo = 0;

#ifdef _WIN32
// IMPLEMENTACIÓN PARA WINDOWS:

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
// IMPLEMENTACIÓN PARA LINUX:
    
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
        if (kill(procesoPar->pid, 0) == 0) {
            kill(procesoPar->pid, SIGTERM);
            
            int status;
            int result = waitpid(procesoPar->pid, &status, WNOHANG);
            
            if (result == 0) {
                result = waitpid(procesoPar->pid, &status, WNOHANG);

                if (result == 0) {
                    kill(procesoPar->pid, SIGKILL);
                    waitpid(procesoPar->pid, &status, 0);
                }
            }
        }
        
        procesoPar->pid = -1;
    }

#endif

    free(procesoPar);

    return E_OK;
}
