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
    
    /* Cerrar tuberías primero para que el hilo de escucha termine */
    if (procesoPar->hTuberiaEntrada != NULL) {
        CloseHandle(procesoPar->hTuberiaEntrada);
        procesoPar->hTuberiaEntrada = NULL;
    }

    if (procesoPar->hTuberiaSalida != NULL) {
        CloseHandle(procesoPar->hTuberiaSalida);
        procesoPar->hTuberiaSalida = NULL;
    }

    /* Esperar a que el hilo de escucha termine (si existe) */
    if (procesoPar->hHiloEscucha != NULL) {
        WaitForSingleObject(procesoPar->hHiloEscucha, 2000);  /* Esperar 2 segundos */
        CloseHandle(procesoPar->hHiloEscucha);
        procesoPar->hHiloEscucha = NULL;
    }

    /* Terminar el proceso hijo */
    if (procesoPar->hProceso != NULL) {
        /* Intentar terminar el proceso suavemente */
        TerminateProcess(procesoPar->hProceso, 0);
        
        /* Esperar a que el proceso termine */
        WaitForSingleObject(procesoPar->hProceso, 2000);  /* Esperar 2 segundos */
        
        /* Cerrar handles */
        CloseHandle(procesoPar->hProceso);
        procesoPar->hProceso = NULL;
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
        /* Enviar señal SIGTERM para terminar suavemente */
        kill(procesoPar->pid, SIGTERM);
        
        /* Esperar a que el proceso hijo termine */
        int status;
        waitpid(procesoPar->pid, &status, 0);
        
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
