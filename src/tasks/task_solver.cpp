#include <HardwareSerial.h>
#include "globals.h"
#include "battery.h"

// Lógica del SOLVER: aplica acciones correctivas según el fallo
void vSolver(int errorID) {
    Serial.printf("[SOLVER] Aplicando estrategia para error ID: %d\n", errorID);

    switch(errorID) {
        case OVERHEAT_ERROR:
            // Entrar en modo enfriamiento (COOLING_MODE)
            currentMode = MODE_COOLING;
            break;
        case COMPONENT_FAIL:
            // reiniciar el componente específico
            break;
        case PART_BROKEN:
            // isolar y excluir de la operación
            break;
    }
}