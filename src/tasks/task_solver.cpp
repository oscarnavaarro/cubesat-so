#include "globals.h"
#include "../include/task_solver.h"

// Lógica del SOLVER: aplica acciones correctivas según el fallo
void vSolver(SolverError_t errorID) {
    Serial.printf("[SOLVER] Aplicando estrategia para error ID: %d\n", errorID);

    switch(errorID) {
        case OVERHEAT_ERROR:
            // Entrar en modo enfriamiento (COOLING_MODE)
            currentMode = MODE_COOLING;
            Serial.println("[SOLVER] OVERHEAT_ERROR: activando MODE_COOLING");
            break;
        case LOW_BATTERY_ERROR:
            currentMode = MODE_LOW_POWER;
            Serial.println("[SOLVER] LOW_BATTERY_ERROR: activando MODE_LOW_POWER");
            break;
        case COMPONENT_FAIL:
            // reiniciar el componente específico
            Serial.println("[SOLVER] COMPONENT_FAIL: marcar componente para reinicio");
            break;
        case PART_BROKEN:
            // isolar y excluir de la operación
            currentMode = MODE_SAFE;
            Serial.println("[SOLVER] PART_BROKEN: activando MODE_SAFE");
            break;
        default:
            currentMode = MODE_SAFE;
            Serial.println("[SOLVER] ERROR no reconocido: fallback a MODE_SAFE");
            break;
    }
}