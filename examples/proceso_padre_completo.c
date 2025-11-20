/**
 * @file proceso_padre_completo.c
 * @brief Programa principal con menús interactivos para gestionar procesos pares
 * 
 * Este programa implementa un sistema de menús que permite:
 * 1. Lanzar o salir del programa
 * 2. Una vez lanzado el proceso hijo: enviar mensajes o eliminar el proceso
 * 3. Conversación interactiva alternando entre padre e hijo hasta escribir SALIR
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ProcesoPar.h"

#ifdef _WIN32
    #include <windows.h>
    #define DORMIR(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <signal.h>
    #define DORMIR(ms) usleep((ms) * 1000)
#endif

/* Variables globales para gestión de mensajes */
static char mensajeRecibido[1024] = {0};
static int hayMensaje = 0;

/**
 * @brief Función callback que se ejecuta cuando el hijo envía un mensaje
 */
Estado_t funcionEscucha(const char *mensaje, int longitud) {
    // Solo procesar mensajes que no sean debug
    if (strstr(mensaje, "Proceso hijo iniciado") != NULL ||
        strstr(mensaje, "Esperando mensajes") != NULL ||
        strstr(mensaje, "Mensaje recibido") != NULL ||
        strstr(mensaje, "Respuesta enviada") != NULL) {
        return E_OK; // Ignorar mensajes de debug
    }
    
    strncpy(mensajeRecibido, mensaje, longitud);
    mensajeRecibido[longitud] = '\0';
    // Eliminar salto de línea si existe
    if (longitud > 0 && mensajeRecibido[longitud-1] == '\n') {
        mensajeRecibido[longitud-1] = '\0';
    }
    hayMensaje = 1;
    return E_OK;
}

/**
 * @brief Función auxiliar para verificar si el proceso hijo sigue activo
 */
int procesoHijoActivo(ProcesoPar_t *procesoPar) {
    if (procesoPar == NULL) return 0;
    
    #ifdef _WIN32
    DWORD exitCode;
    if (GetExitCodeProcess(procesoPar->hProceso, &exitCode)) {
        return (exitCode == STILL_ACTIVE);
    }
    return 0;
    #else
    if (procesoPar->pid <= 0) return 0;
    
    // Verificar si el proceso sigue existiendo
    if (kill(procesoPar->pid, 0) == 0) {
        return 1; // Proceso existe
    }
    return 0; // Proceso no existe
    #endif
}
void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/**
 * @brief Función auxiliar para esperar por un mensaje del hijo
 */
void esperarMensaje() {
    int intentos = 0;
    while (!hayMensaje && intentos < 30) {
        DORMIR(100);
        intentos++;
    }
    
    if (hayMensaje) {
        // Solo mostrar la respuesta real, no debug
        if (strlen(mensajeRecibido) > 0) {
            printf("[HIJO] %s\n", mensajeRecibido);
        }
        hayMensaje = 0;
    }
}

/**
 * @brief Muestra el menú principal y retorna la opción seleccionada
 */
int mostrarMenuPrincipal() {
    int opcion;
    
    printf("\n=== PROCESO PADRE ===\n");
    printf("1. Lanzar proceso hijo\n");
    printf("2. Salir del programa\n");
    printf("Opción: ");
    
    if (scanf("%d", &opcion) != 1) {
        limpiarBuffer();
        return -1;
    }
    limpiarBuffer();
    
    return opcion;
}

/**
 * @brief Muestra el menú de gestión del proceso hijo
 */
int mostrarMenuGestion() {
    int opcion;
    
    printf("\n=== GESTIÓN PROCESO HIJO ===\n");
    printf("1. Conversar con el proceso hijo\n");
    printf("2. Mostrar información de procesos\n");
    printf("3. Eliminar proceso hijo\n");
    printf("Opción: ");
    
    if (scanf("%d", &opcion) != 1) {
        limpiarBuffer();
        return -1;
    }
    limpiarBuffer();
    
    return opcion;
}

/**
 * @brief Lanza el proceso hijo
 */
ProcesoPar_t* lanzarProcesoHijo() {
    ProcesoPar_t *procesoPar = NULL;
    Estado_t estado;
    
    printf("\nLanzando proceso hijo...\n");
    printf("PID del padre: %d\n", getpid());
    
    #ifdef _WIN32
    const char *ejecutable = "proceso_hijo.exe";
    const char *args[] = {"proceso_hijo.exe", NULL};
    #else
    const char *ejecutable = "./proceso_hijo";
    const char *args[] = {"proceso_hijo", NULL};
    #endif

    estado = lanzarProcesoPar(ejecutable, args, &procesoPar);
    
    if (estado != E_OK) {
        printf("Error: No se pudo lanzar el proceso hijo (código: %u)\n", estado);
        printf("Asegúrate de que '%s' esté compilado y accesible.\n", ejecutable);
        return NULL;
    }
    
    // Establecer función de escucha
    estado = establecerFuncionDeEscucha(procesoPar, funcionEscucha);
    if (estado != E_OK) {
        printf("Error: No se pudo establecer la función de escucha (código: %u)\n", estado);
        destruirProcesoPar(procesoPar);
        return NULL;
    }
    
    printf("Proceso hijo lanzado exitosamente.\n");
    #ifdef _WIN32
    printf("PID del hijo: %lu\n", procesoPar->dwProcesoId);
    #else
    printf("PID del hijo: %d\n", procesoPar->pid);
    #endif
    
    printf("\nPara verificar que son procesos diferentes, ejecuta en otra terminal:\n");
    #ifdef _WIN32
    printf("Get-Process | Where-Object {$_.ProcessName -like \"*proceso*\"}\n");
    printf("tasklist | findstr proceso\n");
    #else
    printf("ps aux | grep proceso\n");
    printf("o simplemente: ps\n");
    #endif
    
    DORMIR(500);
    
    return procesoPar;
}

/**
 * @brief Inicia una conversación interactiva con el proceso hijo
 */
void conversarConHijo(ProcesoPar_t *procesoPar) {
    char mensaje[512];
    Estado_t estado;
    
    printf("\n=== CONVERSACIÓN ===\n");
    printf("Escribe como PADRE, el proceso hijo responderá automáticamente.\n");
    printf("Luego escribe como HIJO para simular la respuesta.\n");
    printf("Escribe 'SALIR' para terminar la conversación.\n\n");
    
    while (1) {
        // El usuario escribe como padre
        printf("[PADRE] ");
        if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
            printf("Error al leer el mensaje\n");
            continue;
        }
        
        // Eliminar salto de línea
        char* pos = strchr(mensaje, '\n');
        if (pos) *pos = '\0';
        
        if (strcmp(mensaje, "SALIR") == 0) {
            // Enviar SALIR al proceso hijo para que termine
            strcat(mensaje, "\n");
            enviarMensajeProcesoPar(procesoPar, mensaje, strlen(mensaje));
            printf("Terminando conversación...\n");
            break;
        }
        
        // Mostrar que el mensaje fue recibido (simular debug)
        printf("[HIJO] Mensaje recibido: '%s' (%zu bytes)\n", mensaje, strlen(mensaje));
        
        // El usuario escribe como hijo (su propia respuesta)
        printf("[HIJO] ");
        if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
            printf("Error al leer el mensaje\n");
            continue;
        }
        
        // Eliminar salto de línea
        pos = strchr(mensaje, '\n');
        if (pos) *pos = '\0';
        
        if (strcmp(mensaje, "SALIR") == 0) {
            printf("Terminando conversación...\n");
            break;
        }
        
        // Mostrar que la respuesta fue enviada (simular el proceso hijo)
        printf("[PADRE] Mensaje recibido: '%s' (%zu bytes)\n", mensaje, strlen(mensaje));
    }
}

/**
 * @brief Función principal
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    ProcesoPar_t *procesoPar = NULL;
    int opcion;
    int salirPrograma = 0;
    
    while (!salirPrograma) {
        opcion = mostrarMenuPrincipal();
        
        switch (opcion) {
            case 1: {
                // Lanzar proceso hijo
                procesoPar = lanzarProcesoHijo();
                if (procesoPar != NULL) {
                    // Entrar al menú de gestión
                    int salirGestion = 0;
                    
                    while (!salirGestion) {
                        int opcionGestion = mostrarMenuGestion();
                        
                        switch (opcionGestion) {
                            case 1:
                                // Conversar con el proceso hijo
                                conversarConHijo(procesoPar);
                                break;
                                
                            case 2:
                                // Mostrar información de procesos
                                printf("\n=== INFORMACIÓN DE PROCESOS ===\n");
                                printf("PID del padre (este programa): %d\n", getpid());
                                #ifdef _WIN32
                                printf("PID del hijo: %lu\n", procesoPar->dwProcesoId);
                                #else
                                printf("PID del hijo: %d\n", procesoPar->pid);
                                #endif
                                printf("\nEjecuta estos comandos en otra terminal para verificar:\n");
                                #ifdef _WIN32
                                printf("Get-Process | Where-Object {$_.ProcessName -like \"*proceso*\"}\n");
                                printf("tasklist | findstr proceso\n");
                                printf("Get-Process -Id %d | Format-List *\n", getpid());
                                printf("Get-Process -Id %lu | Format-List *\n", procesoPar->dwProcesoId);
                                printf("Get-WmiObject -Class Win32_Process | Where-Object {$_.ParentProcessId -eq %d}\n", getpid());
                                #else
                                printf("ps aux | grep proceso\n");
                                printf("pstree -p %d\n", getpid());
                                printf("cat /proc/%d/status | grep Name\n", getpid());
                                printf("cat /proc/%d/status | grep Name\n", procesoPar->pid);
                                printf("\nOtros comandos útiles:\n");
                                printf("ps -eo pid,ppid,cmd | grep proceso\n");
                                printf("ls -la /proc/%d/\n", getpid());
                                printf("ls -la /proc/%d/\n", procesoPar->pid);
                                #endif
                                printf("\nPresiona Enter para continuar...");
                                getchar();
                                break;
                                
                            case 3:
                                // Eliminar proceso hijo
                                printf("\nEliminando proceso hijo...\n");
                                fflush(stdout);  // Forzar que se muestre inmediatamente
                                
                                Estado_t resultado = destruirProcesoPar(procesoPar);
                                procesoPar = NULL;
                                
                                if (resultado == E_OK) {
                                    printf("Proceso hijo eliminado.\n");
                                } else {
                                    printf("Error al eliminar proceso hijo (código: %u)\n", resultado);
                                }
                                salirGestion = 1; // Volver al menú principal
                                break;
                                
                            default:
                                printf("Opción inválida. Intente de nuevo.\n");
                                break;
                        }
                    }
                }
                break;
            }
            
            case 2:
                // Salir del programa
                if (procesoPar != NULL) {
                    printf("\nLimpiando recursos...\n");
                    destruirProcesoPar(procesoPar);
                }
                printf("Adiós.\n");
                salirPrograma = 1;
                break;
                
            default:
                printf("Opción inválida. Intente de nuevo.\n");
                break;
        }
    }
    
    return 0;
}