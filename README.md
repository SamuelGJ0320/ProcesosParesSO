# Práctica Procesos Pares

**Asignatura:** Sistemas Operativos  
**Proyecto:** Biblioteca para comunicación bidireccional entre procesos

## Descripción

Este proyecto implementa una **biblioteca de procesos pares** que permite a un proceso padre crear y comunicarse bidireccionalmente con un proceso hijo usando tuberías.

### Funcionalidades principales:
- **Crear procesos hijo** desde un proceso padre
- **Comunicación full-duplex** (A→B y B→A) mediante tuberías
- **Función de escucha** para recibir mensajes del proceso hijo
- **Gestión completa** del ciclo de vida del proceso (crear/destruir)
- **Multiplataforma** (Windows y Linux)

### API implementada:
```c
Estado_t lanzarProcesoPar(...)           // Crear proceso hijo
Estado_t enviarMensajeProcesoPar(...)    // Enviar mensaje al hijo  
Estado_t establecerFuncionDeEscucha(...) // Recibir mensajes del hijo
Estado_t destruirProcesoPar(...)         // Terminar proceso hijo
```

## Compilación y Ejecución

### Linux

```bash
# Compilar usando Makefile
make

# Ejecutar el programa
cd examples
./proceso_padre_completo
```

### Windows (desde WSL)

```bash
# Compilar usando script de Windows
chmod +x compilar_windows.sh
./compilar_windows.sh

# Ejecutar desde PowerShell de Windows
cd examples
./proceso_padre_completo.exe


## Uso del Programa

1. **Menú Principal:**
   - Opción 1: Lanzar proceso hijo
   - Opción 2: Salir

2. **Menú de Gestión (con proceso hijo activo):**
   - Opción 1: **Conversar** - Comunicación interactiva bidireccional
   - Opción 2: **Información** - Ver PIDs de procesos padre e hijo  
   - Opción 3: **Eliminar** - Terminar proceso hijo

3. **Conversación:**
   - Escribe como **[PADRE]** → mensaje va al proceso hijo
   - Escribe como **[HIJO]** → respuesta del proceso hijo al padre
   - Escribe `SALIR` para terminar la conversación


## Verificación

Para verificar que son procesos diferentes ejecuta en otra terminal:

**Linux:**
```bash
ps aux | grep proceso
pstree -p $(pgrep proceso_padre_completo)
watch "ps aux | grep proceso"
```

**Windows:**
```powershell
Get-Process | Where-Object {$_.ProcessName -like "*proceso*"}
tasklist | findstr proceso
while ($true) { 
    Clear-Host
    Write-Host "=== PROCESOS EN TIEMPO REAL ===" -ForegroundColor Green
    Get-Process | Where-Object {$_.ProcessName -like "*proceso*"} | Format-Table Id, ProcessName, CPU, WorkingSet -AutoSize
    Start-Sleep 2 
}
```

## Requerimientos

- **Linux:** GCC, pthread
- **Windows:** MinGW-w64 (se instala automáticamente con el script)

- **WSL:** Para compilación cruzada Windows desde Linux
