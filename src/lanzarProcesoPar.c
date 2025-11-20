/**
* @file
* @brief
*/

#include "../include/ProcesoPar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    // Implementación para Windows
    #include <windows.h>
#else
    // Implementación para Linux
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pthread.h>
#endif

// Declaración de función para el hilo de escucha
#ifdef _WIN32
DWORD WINAPI hiloEscucha(LPVOID param);
#else
void* hiloEscucha(void* param);
#endif

/**
* @brief
*/
Estado_t lanzarProcesoPar(
    const char *nombreArchivoEjecutable,
    const char **listaLineaComando,
    ProcesoPar_t **procesoPar
) {
    if (nombreArchivoEjecutable == NULL || procesoPar == NULL) {
        return E_PAR_INC;
    }

    ProcesoPar_t *pp = (ProcesoPar_t*)malloc(sizeof(ProcesoPar_t));
    if (pp == NULL) {
        return E_NO_MEMORIA;
    }

    pp->funcionEscucha = NULL;
    pp->activo = 0;

#ifdef _WIN32
// Implementación para Windows
    
    SECURITY_ATTRIBUTES sa;
    HANDLE hTuberiaLecturaHijo, hTuberiaEscrituraHijo;
    HANDLE hTuberiaLecturaPadre, hTuberiaEscrituraPadre;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL exito;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Crear tubería 1: Padre escribe -> Hijo lee (Salida del padre)
    if (!CreatePipe(&hTuberiaLecturaHijo, &hTuberiaEscrituraPadre, &sa, 0)) {
        free(pp);
        return E_CREAR_PIPE;
    }

    if (!SetHandleInformation(hTuberiaEscrituraPadre, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        free(pp);
        return E_CREAR_PIPE;
    }

    // Crear tubería 2: Hijo escribe -> Padre lee (Entrada al padre)
    if (!CreatePipe(&hTuberiaLecturaPadre, &hTuberiaEscrituraHijo, &sa, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        free(pp);
        return E_CREAR_PIPE;
    }

    if (!SetHandleInformation(hTuberiaLecturaPadre, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        CloseHandle(hTuberiaLecturaPadre);
        CloseHandle(hTuberiaEscrituraHijo);
        free(pp);
        return E_CREAR_PIPE;
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hTuberiaEscrituraHijo;
    si.hStdOutput = hTuberiaEscrituraHijo;
    si.hStdInput = hTuberiaLecturaHijo;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Construir línea de comandos
    char comandoCompleto[1024] = "";
    if (listaLineaComando != NULL) {
        int i = 0;
        while (listaLineaComando[i] != NULL) {
            if (i > 0) strcat(comandoCompleto, " ");
            strcat(comandoCompleto, listaLineaComando[i]);
            i++;
        }
    } else {
        strcpy(comandoCompleto, nombreArchivoEjecutable);
    }

    // Crear el proceso hijo
    ZeroMemory(&pi, sizeof(pi));
    exito = CreateProcessA(
        nombreArchivoEjecutable,
        comandoCompleto,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!exito) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        CloseHandle(hTuberiaLecturaPadre);
        CloseHandle(hTuberiaEscrituraHijo);
        free(pp);
        return E_CREAR_PROCESO;
    }

    CloseHandle(hTuberiaLecturaHijo);
    CloseHandle(hTuberiaEscrituraHijo);

    pp->hProceso = pi.hProcess;
    pp->hHilo = pi.hThread;
    pp->dwProcesoId = pi.dwProcessId;
    pp->hTuberiaEntrada = hTuberiaLecturaPadre;  /* Padre LEE desde aquí */
    pp->hTuberiaSalida = hTuberiaEscrituraPadre; /* Padre ESCRIBE aquí */
    pp->hHiloEscucha = NULL;
    pp->activo = 1;

#else
// IMPLEMENTACIÓN PARA LINUX:

    if (pipe(pp->pipeEntrada) == -1) {
        free(pp);
        return E_CREAR_PIPE;
    }

    if (pipe(pp->pipeSalida) == -1) {
        close(pp->pipeEntrada[0]);
        close(pp->pipeEntrada[1]);
        free(pp);
        return E_CREAR_PIPE;
    }

    // Crear el proceso hijo
    pp->pid = fork();

    if (pp->pid == -1) {
        close(pp->pipeEntrada[0]);
        close(pp->pipeEntrada[1]);
        close(pp->pipeSalida[0]);
        close(pp->pipeSalida[1]);
        free(pp);
        return E_CREAR_PROCESO;
    }

    if (pp->pid == 0) {
        
        close(pp->pipeEntrada[0]);
        close(pp->pipeSalida[1]);

        dup2(pp->pipeSalida[0], STDIN_FILENO);
        close(pp->pipeSalida[0]);

        dup2(pp->pipeEntrada[1], STDOUT_FILENO);
        close(pp->pipeEntrada[1]);

        // Ejecutar el programa hijo
        if (listaLineaComando != NULL) {
            execvp(nombreArchivoEjecutable, (char* const*)listaLineaComando);
        } else {
            char* args[] = {(char*)nombreArchivoEjecutable, NULL};
            execvp(nombreArchivoEjecutable, args);
        }

        // Si llegamos aquí, execvp falló
        perror("execvp");
        exit(1);
    } else {

        close(pp->pipeEntrada[1]);
        close(pp->pipeSalida[0]);

        pp->activo = 1;
    }
#endif

    *procesoPar = pp;
    return E_OK;
}
