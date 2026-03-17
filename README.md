# My CubeSat Application

Este proyecto es una aplicación para un satélite en miniatura (CubeSat) que gestiona diferentes modos de operación y supervisa el estado de sus subsistemas. Utiliza FreeRTOS para la gestión de tareas y simula el nivel de batería.

## Estructura del Proyecto

El proyecto está organizado de la siguiente manera:

```
my-cubesat-app
├── include
│   ├── globals.h          # Definiciones de variables globales y constantes
│   ├── modes.h            # Definición del tipo enumerado SatMode_t
│   └── battery.h          # Declaraciones de funciones de gestión de batería
├── src
│   ├── main.cpp           # Punto de entrada de la aplicación
│   ├── globals.cpp        # Implementación de variables globales
│   ├── battery.cpp        # Implementación de funciones de batería
│   ├── tasks
│   │   ├── task_monitor.cpp          # Implementación de la tarea de monitoreo
│   │   ├── task_monitor.h            # Declaración de la tarea de monitoreo
│   │   ├── task_checking.cpp         # Implementación de la tarea de verificación
│   │   ├── task_checking.h           # Declaración de la tarea de verificación
│   │   ├── task_mode_manager.cpp     # Implementación de la tarea de gestión de modos
│   │   ├── task_mode_manager.h       # Declaración de la tarea de gestión de modos
│   │   ├── task_leds_pulsing.cpp     # Implementación de la tarea de parpadeo de LEDs
│   │   ├── task_leds_pulsing.h       # Declaración de la tarea de parpadeo de LEDs
│   │   ├── task_post_deployment.cpp  # Implementación de la tarea de post-despliegue
│   │   └── task_post_deployment.h    # Declaración de la tarea de post-despliegue
├── platformio.ini         # Configuración para PlatformIO
└── README.md              # Documentación del proyecto
```

## Instrucciones de Uso

1. **Configuración del Entorno**: Asegúrate de tener PlatformIO instalado y configurado en tu entorno de desarrollo.
2. **Compilación**: Abre el proyecto en PlatformIO y compila el código. Asegúrate de que no haya errores de compilación.
3. **Carga en el Dispositivo**: Conecta tu dispositivo y carga el firmware compilado.
4. **Monitoreo**: Utiliza el monitor serie para observar el estado del satélite y los mensajes de depuración.

## Detalles de Implementación

- **Gestión de Modos**: El satélite puede operar en varios modos (Inicialización, Despliegue, Nominal, Bajo Consumo, Seguro) y cambia de modo según el nivel de batería.
- **Monitoreo de Subsistemas**: Se implementa una tarea de monitoreo que verifica el estado de otros subsistemas y reporta el uso de la pila.
- **Simulación de Batería**: Se simula el nivel de batería para probar la lógica de gestión de modos.

## Contribuciones

Las contribuciones son bienvenidas. Si deseas mejorar el proyecto, por favor abre un issue o un pull request en el repositorio.

## Licencia

Este proyecto está bajo la Licencia MIT.