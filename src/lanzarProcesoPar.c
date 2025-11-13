/**
 * @file lanzarProcesoPar.c
 * @brief Implementación de la función para crear un proceso par
 */

#include "../include/ProcesoPar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    /* Implementación para Windows */
    #include <windows.h>
#else
    /* Implementación para Linux */
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pthread.h>
#endif

/* Declaración de función auxiliar para el hilo de escucha */
#ifdef _WIN32
DWORD WINAPI hiloEscucha(LPVOID param);
#else
void* hiloEscucha(void* param);
#endif

/**
 * @brief Lanza un nuevo proceso par (proceso hijo)
 */
Estado_t lanzarProcesoPar(
    const char *nombreArchivoEjecutable,
    const char **listaLineaComando,
    ProcesoPar_t **procesoPar
) {
    /* Validar parámetros */
    if (nombreArchivoEjecutable == NULL || procesoPar == NULL) {
        return E_PAR_INC;
    }

    /* Asignar memoria para la estructura ProcesoPar_t */
    ProcesoPar_t *pp = (ProcesoPar_t*)malloc(sizeof(ProcesoPar_t));
    if (pp == NULL) {
        return E_NO_MEMORIA;
    }

    /* Inicializar campos comunes */
    pp->funcionEscucha = NULL;
    pp->activo = 0;

#ifdef _WIN32
    /* ========================================
     * IMPLEMENTACIÓN PARA WINDOWS
     * ======================================== */
    
    SECURITY_ATTRIBUTES sa;
    HANDLE hTuberiaLecturaHijo, hTuberiaEscrituraHijo;
    HANDLE hTuberiaLecturaPadre, hTuberiaEscrituraPadre;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL exito;

    /* Configurar atributos de seguridad para que los handles sean heredables */
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    /* Crear tubería 1: Padre escribe -> Hijo lee (Salida del padre) */
    if (!CreatePipe(&hTuberiaLecturaHijo, &hTuberiaEscrituraPadre, &sa, 0)) {
        free(pp);
        return E_CREAR_PIPE;
    }

    /* Asegurar que el extremo de escritura del padre no sea heredable */
    if (!SetHandleInformation(hTuberiaEscrituraPadre, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        free(pp);
        return E_CREAR_PIPE;
    }

    /* Crear tubería 2: Hijo escribe -> Padre lee (Entrada al padre) */
    if (!CreatePipe(&hTuberiaLecturaPadre, &hTuberiaEscrituraHijo, &sa, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        free(pp);
        return E_CREAR_PIPE;
    }

    /* Asegurar que el extremo de lectura del padre no sea heredable */
    if (!SetHandleInformation(hTuberiaLecturaPadre, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        CloseHandle(hTuberiaLecturaPadre);
        CloseHandle(hTuberiaEscrituraHijo);
        free(pp);
        return E_CREAR_PIPE;
    }

    /* Configurar STARTUPINFO */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hTuberiaEscrituraHijo;
    si.hStdOutput = hTuberiaEscrituraHijo;
    si.hStdInput = hTuberiaLecturaHijo;
    si.dwFlags |= STARTF_USESTDHANDLES;

    /* Construir línea de comandos */
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

    /* Crear el proceso hijo */
    ZeroMemory(&pi, sizeof(pi));
    exito = CreateProcessA(
        nombreArchivoEjecutable,  /* Nombre del módulo */
        comandoCompleto,          /* Línea de comandos */
        NULL,                     /* Atributos de seguridad del proceso */
        NULL,                     /* Atributos de seguridad del hilo */
        TRUE,                     /* Heredar handles */
        0,                        /* Flags de creación */
        NULL,                     /* Usar ambiente del padre */
        NULL,                     /* Usar directorio del padre */
        &si,                      /* STARTUPINFO */
        &pi                       /* PROCESS_INFORMATION */
    );

    if (!exito) {
        CloseHandle(hTuberiaLecturaHijo);
        CloseHandle(hTuberiaEscrituraPadre);
        CloseHandle(hTuberiaLecturaPadre);
        CloseHandle(hTuberiaEscrituraHijo);
        free(pp);
        return E_CREAR_PROCESO;
    }

    /* Cerrar los handles que el padre no necesita */
    CloseHandle(hTuberiaLecturaHijo);
    CloseHandle(hTuberiaEscrituraHijo);

    /* Guardar información en la estructura */
    pp->hProceso = pi.hProcess;
    pp->hHilo = pi.hThread;
    pp->dwProcesoId = pi.dwProcessId;
    pp->hTuberiaEntrada = hTuberiaLecturaPadre;  /* Padre LEE desde aquí */
    pp->hTuberiaSalida = hTuberiaEscrituraPadre; /* Padre ESCRIBE aquí */
    pp->hHiloEscucha = NULL;
    pp->activo = 1;

#else
    /* ========================================
     * IMPLEMENTACIÓN PARA LINUX
     * ======================================== */
    
    /* Crear tuberías
     * pipeEntrada: el hijo ESCRIBE aquí, el padre LEE desde aquí
     * pipeSalida: el padre ESCRIBE aquí, el hijo LEE desde aquí
     */
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

    /* Crear el proceso hijo con fork() */
    pp->pid = fork();

    if (pp->pid == -1) {
        /* Error al crear proceso */
        close(pp->pipeEntrada[0]);
        close(pp->pipeEntrada[1]);
        close(pp->pipeSalida[0]);
        close(pp->pipeSalida[1]);
        free(pp);
        return E_CREAR_PROCESO;
    }

    if (pp->pid == 0) {
        /* ===== CÓDIGO DEL PROCESO HIJO ===== */
        
        /* Cerrar extremos que el hijo no usa */
        close(pp->pipeEntrada[0]);  /* El hijo no lee de pipeEntrada */
        close(pp->pipeSalida[1]);   /* El hijo no escribe en pipeSalida */

        /* Redirigir stdin al extremo de lectura de pipeSalida */
        dup2(pp->pipeSalida[0], STDIN_FILENO);
        close(pp->pipeSalida[0]);

        /* Redirigir stdout al extremo de escritura de pipeEntrada */
        dup2(pp->pipeEntrada[1], STDOUT_FILENO);
        close(pp->pipeEntrada[1]);

        /* Ejecutar el programa hijo */
        if (listaLineaComando != NULL) {
            execvp(nombreArchivoEjecutable, (char* const*)listaLineaComando);
        } else {
            char* args[] = {(char*)nombreArchivoEjecutable, NULL};
            execvp(nombreArchivoEjecutable, args);
        }

        /* Si llegamos aquí, execvp falló */
        perror("execvp");
        exit(1);
    } else {
        /* ===== CÓDIGO DEL PROCESO PADRE ===== */
        
        /* Cerrar extremos que el padre no usa */
        close(pp->pipeEntrada[1]);  /* El padre no escribe en pipeEntrada */
        close(pp->pipeSalida[0]);   /* El padre no lee de pipeSalida */

        pp->activo = 1;
    }
#endif

    /* Retornar el puntero al proceso par creado */
    *procesoPar = pp;
    return E_OK;
}
