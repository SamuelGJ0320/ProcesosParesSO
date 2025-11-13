/**
 * @file proceso_padre.c
 * @brief Programa de ejemplo que usa la biblioteca ProcesoPar
 * 
 * Este programa demuestra cómo:
 * - Lanzar un proceso hijo
 * - Enviar mensajes al proceso hijo
 * - Recibir y procesar mensajes del hijo mediante un callback
 * - Destruir el proceso cuando ya no se necesita
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
    #define DORMIR(ms) usleep((ms) * 1000)
#endif

/* Variable global para controlar el flujo */
int mensajesRecibidos = 0;

/**
 * @brief Función callback que se ejecuta cuando el hijo envía un mensaje
 */
Estado_t funcionEscucha(const char *mensaje, int longitud) {
    printf("[PADRE] <<<< Mensaje recibido del hijo (%d bytes): '%s'\n", longitud, mensaje);
    fflush(stdout);
    mensajesRecibidos++;
    return E_OK;
}

/**
 * @brief Función auxiliar para enviar un mensaje y reportar el resultado
 */
void enviarYReportar(ProcesoPar_t *pp, const char *mensaje) {
    printf("[PADRE] >>>> Enviando: '%s'\n", mensaje);
    fflush(stdout);
    
    Estado_t estado = enviarMensajeProcesoPar(pp, mensaje, strlen(mensaje));
    
    if (estado != E_OK) {
        fprintf(stderr, "[PADRE] Error al enviar mensaje: código %u\n", estado);
    }
}

int main(int argc, char *argv[]) {
    (void)argc;  /* Parámetro no usado */
    (void)argv;  /* Parámetro no usado */
    ProcesoPar_t *procesoPar = NULL;
    Estado_t estado;
    
    printf("==============================================\n");
    printf("  EJEMPLO DE USO DE BIBLIOTECA PROCESOPAR\n");
    printf("==============================================\n\n");

    /* ===== 1. LANZAR EL PROCESO HIJO ===== */
    printf("[PASO 1] Lanzando proceso hijo...\n");
    
    #ifdef _WIN32
    /* En Windows, especificar la ruta completa o asegurarse de que esté en PATH */
    const char *ejecutable = "proceso_hijo.exe";
    const char *args[] = {"proceso_hijo.exe", NULL};
    #else
    /* En Linux, usar el ejecutable compilado */
    const char *ejecutable = "./proceso_hijo";
    const char *args[] = {"proceso_hijo", NULL};
    #endif

    estado = lanzarProcesoPar(ejecutable, args, &procesoPar);
    
    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] No se pudo lanzar el proceso hijo. Código: %u\n", estado);
        fprintf(stderr, "        Asegúrate de que '%s' esté compilado y en la ubicación correcta.\n", ejecutable);
        return 1;
    }
    
    printf("[OK] Proceso hijo lanzado exitosamente!\n\n");

    /* ===== 2. ESTABLECER FUNCIÓN DE ESCUCHA ===== */
    printf("[PASO 2] Estableciendo función de escucha...\n");
    
    estado = establecerFuncionDeEscucha(procesoPar, funcionEscucha);
    
    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] No se pudo establecer la función de escucha. Código: %u\n", estado);
        destruirProcesoPar(procesoPar);
        return 1;
    }
    
    printf("[OK] Función de escucha establecida!\n\n");

    /* Pequeña pausa para dar tiempo al hilo de escucha */
    DORMIR(500);

    /* ===== 3. ENVIAR MENSAJES AL HIJO ===== */
    printf("[PASO 3] Enviando mensajes al proceso hijo...\n");
    printf("--------------------------------------------------\n");

    /* Mensaje 1: Saludo */
    enviarYReportar(procesoPar, "HOLA\n");
    DORMIR(1000);  /* Esperar respuesta */

    /* Mensaje 2: Ping */
    enviarYReportar(procesoPar, "PING\n");
    DORMIR(1000);

    /* Mensaje 3: Mensaje personalizado */
    enviarYReportar(procesoPar, "Este es un mensaje de prueba\n");
    DORMIR(1000);

    /* Mensaje 4: Otro ping */
    enviarYReportar(procesoPar, "PING\n");
    DORMIR(1000);

    printf("--------------------------------------------------\n");
    printf("[INFO] Total de mensajes enviados: 4\n");
    printf("[INFO] Total de respuestas recibidas: %d\n\n", mensajesRecibidos);

    /* ===== 4. TERMINAR EL PROCESO HIJO ===== */
    printf("[PASO 4] Enviando comando de salida...\n");
    enviarYReportar(procesoPar, "SALIR\n");
    DORMIR(1000);

    /* ===== 5. DESTRUIR EL PROCESO PAR ===== */
    printf("\n[PASO 5] Destruyendo proceso par y liberando recursos...\n");
    
    estado = destruirProcesoPar(procesoPar);
    
    if (estado != E_OK) {
        fprintf(stderr, "[ERROR] Error al destruir el proceso par. Código: %u\n", estado);
        return 1;
    }
    
    printf("[OK] Proceso par destruido correctamente!\n\n");

    /* ===== RESUMEN ===== */
    printf("==============================================\n");
    printf("  DEMOSTRACIÓN COMPLETADA EXITOSAMENTE\n");
    printf("==============================================\n");
    printf("Mensajes enviados: 5 (incluyendo SALIR)\n");
    printf("Respuestas recibidas: %d\n", mensajesRecibidos);
    printf("\nLa biblioteca ProcesoPar está funcionando correctamente.\n");

    #ifdef _WIN32
    /* En Windows, mostrar el log del proceso hijo */
    printf("\n");
    printf("==============================================\n");
    printf("  LOG DEL PROCESO HIJO (hijo_debug.log)\n");
    printf("==============================================\n");
    FILE* log = fopen("hijo_debug.log", "r");
    if (log) {
        char linea[256];
        while (fgets(linea, sizeof(linea), log)) {
            printf("%s", linea);
        }
        fclose(log);
        /* Eliminar el archivo de log */
        remove("hijo_debug.log");
    } else {
        printf("[ADVERTENCIA] No se pudo abrir el archivo de log del hijo\n");
    }
    printf("==============================================\n");
    #endif

    return 0;
}
