// SOLVER - Diagnóstico y Clasificación de Errores
// Con la arquitectura FDIR/Mode Manager, la decisión de modo es responsabilidad exclusiva
// del Mode Manager. Esta tarea queda reducida a un clasificador de errores que
// actualiza healthError según el estado de salud, para enriquecer la telemetría.

#include "globals.h"
#include "../include/task_solver.h"
#include "../include/sat_config.h"

void vTaskSolver(void *pvParameters) {
    for (;;) {
        // Clasificar el error en función del modo actual y el estado de salud.
        // Solo actualiza healthError para fines de telemetría; no cambia currentMode ni healthStatus.
        if (healthStatus == HEALTH_NOK && healthError == NO_ERROR) {
            // Si el Mode Manager puso HEALTH_NOK pero no hay error clasificado aún,
            // intentamos inferirlo del modo actual.
            switch (currentMode) {
                case MODE_SAFE:
                    // No podemos saber el motivo exacto sin los valores de sensor aquí,
                    // pero el FDIR ya lo habrá detallado en su log.
                    healthError = UNKNOWN_ERROR;
                    break;
                default:
                    break;
            }
        } else if (healthStatus == HEALTH_OK) {
            healthError = NO_ERROR;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}