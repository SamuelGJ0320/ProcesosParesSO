/**
 * @file 
 * @brief
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

static char mensajeRecibido[1024] = {0};
static int hayMensaje = 0;

/**
 * @brief
 */
Estado_t funcionEscucha(const char *mensaje, int longitud) {
    strncpy(mensajeRecibido, mensaje, longitud);
    mensajeRecibido[longitud] = '\0';
    
    if (longitud > 0 && mensajeRecibido[longitud-1] == '\n') {
        mensajeRecibido[longitud-1] = '\0';
        longitud--;
    }
    
    // Mostrar lo que envía el hijo
    printf("%s\n", mensajeRecibido);
    fflush(stdout);
    
    hayMensaje = 1;
    return E_OK;
}

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/**
 * @brief
 */
void esperarMensaje() {
    int intentos = 0;
    hayMensaje = 0; // Reset del flag
    
    while (!hayMensaje && intentos < 50) { // Más tiempo de espera
        DORMIR(100);
        intentos++;
    }
    
    if (hayMensaje) {
        // Mostrar la respuesta del proceso hijo
        if (strlen(mensajeRecibido) > 0) {
            printf("[PADRE] Mensaje recibido: '%s' (%zu bytes)\n", mensajeRecibido, strlen(mensajeRecibido));
        }
        hayMensaje = 0;
    }
}

/**
 * @brief
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
 * @brief 
 */
int mostrarMenuGestion() {
    int opcion;
    
    printf("\n=== GESTIÓN CON EL PROCESO HIJO ===\n");
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
 * @brief
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
    printf("Escribe como PADRE, luego escribe como HIJO para responder.\n");
    printf("La comunicación entre procesos es 100%% real y bidireccional.\n");
    printf("Escribe 'SALIR' para terminar la conversación.\n\n");
    
    while (1) {
        printf("[PADRE] ");
        if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
            printf("Error al leer el mensaje\n");
            continue;
        }
        
        char* pos = strchr(mensaje, '\n');
        if (pos) *pos = '\0';
        
        if (strcmp(mensaje, "SALIR") == 0) {
            printf("Terminando conversación...\n");
            break;
        }
        
        // Enviar mensaje al proceso hijo
        char mensajeConSalto[514];
        snprintf(mensajeConSalto, sizeof(mensajeConSalto), "%s\n", mensaje);
        estado = enviarMensajeProcesoPar(procesoPar, mensajeConSalto, strlen(mensajeConSalto));
        
        if (estado != E_OK) {
            printf("Error al enviar mensaje al proceso hijo\n");
            break;
        }
        
        DORMIR(50);
        
        // El usuario escribe como hijo
        printf("[HIJO] ");
        if (fgets(mensaje, sizeof(mensaje), stdin) == NULL) {
            printf("Error al leer el mensaje\n");
            continue;
        }
        
        pos = strchr(mensaje, '\n');
        if (pos) *pos = '\0';
        
        if (strcmp(mensaje, "SALIR") == 0) {
            printf("Terminando conversación...\n");
            break;
        }
        
        // Enviar comando al hijo para que envíe la respuesta al padre
        char comandoEnvio[520];
        snprintf(comandoEnvio, sizeof(comandoEnvio), "ENVIAR:%s\n", mensaje);
        estado = enviarMensajeProcesoPar(procesoPar, comandoEnvio, strlen(comandoEnvio));
        
        if (estado != E_OK) {
            printf("Error al enviar comando de respuesta al hijo\n");
            break;
        }
        
        esperarMensaje();
    }
}

/**
 * @brief
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
                                fflush(stdout);
                                
                                char terminarMsg[] = "TERMINAR_PROCESO\n";
                                enviarMensajeProcesoPar(procesoPar, terminarMsg, strlen(terminarMsg));
                                DORMIR(200);
                                
                                Estado_t resultado = destruirProcesoPar(procesoPar);
                                procesoPar = NULL;
                                
                                if (resultado == E_OK) {
                                    printf("Proceso hijo eliminado.\n");
                                } else {
                                    printf("Error al eliminar proceso hijo (código: %u)\n", resultado);
                                }
                                salirGestion = 1;
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