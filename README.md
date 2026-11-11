# ContenedoresContraReloj

Trabajo Práctico Final — Algoritmos y Estructuras de Datos

**Autores:** Martin Palacios, Eduardo Hidalgo, Erin Gomez

---

## Descripción

ContenedoresContraReloj es un juego de gestión portuaria por turnos desarrollado en C. El jugador asume el rol de operador de un puerto y debe coordinar la descarga de buques, el almacenamiento de contenedores en zonas y la entrega a camiones, todo dentro de un tiempo limitado.

Cada jornada es generada aleatoriamente a partir de un archivo de configuración, lo que garantiza partidas únicas en cada ejecución.

---

## Requisitos

- **IDE:** Code::Blocks 20.03 o superior
- **Compilador:** GCC / MinGW (incluido con Code::Blocks)
- **Sistema operativo:** Windows

---

## Compilación y ejecución

1. Abrir el archivo `ContenedoresContraReloj.cbp` con Code::Blocks
2. Seleccionar **Build → Rebuild** (o presionar `Ctrl+F11`)
3. Ejecutar con **Build → Run** (o presionar `F9`)

> No es necesario compilar manualmente desde la consola. El proyecto ya tiene configurados los directorios de búsqueda de cabeceras.

---

## Configuración (`config.txt`)

Antes de iniciar una jornada el programa lee `config.txt`, que define los parámetros del puerto. El archivo debe tener exactamente 10 campos en este orden:

```
duracion_jornada_minutos: 50
cantidad_muelles: 2
cantidad_zonas_almacenamiento: 3
capacidad_pila: 5
maximo_buques: 4
maximo_contenedores_por_buque: 4
maximo_camiones: 4
tiempo_descarga_contenedor: 2
tiempo_reubicacion_contenedor: 1
tiempo_carga_camion: 2
```

| Campo | Descripción | Rango |
|---|---|---|
| `duracion_jornada_minutos` | Duración total de la jornada | 10 – 100 |
| `cantidad_muelles` | Muelles disponibles para atracar buques | ≥ 1 |
| `cantidad_zonas_almacenamiento` | Zonas de apilado de contenedores | ≥ 1 |
| `capacidad_pila` | Contenedores máximos por zona | ≥ 1 |
| `maximo_buques` | Máximo de buques que puede llegar en la jornada | 1 – 999 |
| `maximo_contenedores_por_buque` | Máximo de contenedores por buque | 1 – 99 |
| `maximo_camiones` | Máximo de camiones que puede llegar en la jornada | 1 – 999 |
| `tiempo_descarga_contenedor` | Costo en minutos de la operación DES | 1 – duración |
| `tiempo_reubicacion_contenedor` | Costo en minutos de la operación REU | 1 – duración |
| `tiempo_carga_camion` | Costo en minutos de la operación ENT | 1 – duración |

> **Recomendación:** usar `cantidad_zonas * capacidad_pila >= maximo_contenedores_por_buque` para evitar escenarios imposibles de completar.

---

## Cómo jugar

Al iniciar una jornada el sistema genera automáticamente el escenario (`puerto.txt`) y muestra la pantalla de ayuda con todas las operaciones disponibles.

En cada turno se debe ingresar una operación con el formato indicado:

### Operaciones

| Operación | Sintaxis | Descripción |
|---|---|---|
| **DES** | `DES Mx Zy` | Descarga el próximo contenedor del buque atracado en el muelle `Mx` y lo apila en la zona `Zy` |
| **REU** | `REU Zx Zy` | Mueve el contenedor del tope de la zona `Zx` al tope de la zona `Zy` (las zonas deben ser distintas) |
| **ENT** | `ENT` | Entrega al camión del frente de la cola el contenedor que solicita (debe estar en el tope de alguna zona) |
| **ESP** | `ESP` | Espera 1 minuto sin realizar otra operación |
| **VER** | `VER` | Muestra el estado completo del puerto (muelles, zonas, cola de camiones y puntuación provisoria) sin consumir tiempo |

> Las operaciones se ingresan siempre en **mayúsculas**.

### Flujo automático

Cuando no hay ninguna operación posible (DES, REU ni ENT), el sistema avanza el reloj automáticamente al próximo arribo de buque o camión. No hace falta escribir ESP manualmente en esos casos.

---

## Fin de jornada

La jornada puede terminar por tres motivos:

| Motivo | Condición |
|---|---|
| **Jornada finalizada exitosamente** | No quedan buques con carga, ni camiones esperando, ni eventos futuros |
| **Tiempo agotado** | El reloj alcanzó la duración configurada |
| **Sin operaciones posibles** | No es posible ejecutar ninguna operación y no hay más eventos por procesar |

Al finalizar se muestra el historial completo de operaciones y el resumen final con la puntuación.

---

## Puntuación

```
Puntuacion = (10 x contenedores entregados) + (5 x buques descargados) - (2 x camiones pendientes)
```

| Concepto | Puntos |
|---|---|
| Contenedor entregado a un camión | +10 |
| Buque completamente descargado | +5 |
| Camión en cola al cierre de la jornada | -2 |

---

## Estructura del proyecto

```
ContenedoresContraReloj/
├── include/
│   ├── tda/          → nodo.h, pila.h, cola.h, lista.h, abb.h
│   ├── dominio/      → jornada.h, escenario.h
│   ├── sesion/       → operador.h
│   ├── util.h        → leer_entero(), constantes BASE_DECIMAL, MAX_BUFFER
│   ├── menu.h        → menu(), seleccionar_opcion(), validar_opcion()
│   └── aplicacion.h  → tConfiguracion, procesar_configuracion(), generar_archivo_puerto()
├── src/
│   ├── tda/          → pila.c, cola.c, lista.c, abb.c
│   ├── dominio/      → jornada.c, escenario.c
│   ├── sesion/       → operador.c
│   ├── util.c
│   ├── menu.c
│   └── aplicacion.c
├── main.c
├── config.txt        → configuracion del puerto (editable)
├── puerto.txt        → escenario generado automaticamente (no editar)
├── operadores.bin    → registro de operadores (generado al jugar)
├── jornadas.bin      → historial de jornadas (generado al jugar)
└── ContenedoresContraReloj.cbp
```

### Módulos principales

| Módulo | Responsabilidad |
|---|---|
| `main.c` | Punto de entrada, loop del menú principal |
| `aplicacion` | Lectura y validación de `config.txt`, generación de `puerto.txt` |
| `menu` | Presentación del menú y lectura de opciones |
| `jornada` | Lógica completa de la jornada (operaciones, reloj, estado, visualización) |
| `escenario` | Lectura de `puerto.txt` y construcción de buques/camiones |
| `operador` | Sesión de usuario, ranking, persistencia en archivos binarios |
| `util` | Conversión estricta de cadenas a enteros |
| `tda/` | Implementaciones genéricas de pila, cola, lista enlazada y ABB |

---

## Archivos generados al jugar

| Archivo | Contenido |
|---|---|
| `puerto.txt` | Escenario de la jornada (se sobreescribe en cada partida) |
| `operadores.bin` | Registro de todos los operadores (se acumula) |
| `jornadas.bin` | Historial de todas las jornadas jugadas (se acumula) |

---

## Notas técnicas

- Los TDAs (pila, cola, lista, ABB) son **genéricos**: almacenan cualquier tipo de dato usando `void*` y `memcpy`.
- La gestión de memoria es completamente manual (`malloc`/`free`). No se usan VLAs.
- La lectura de archivos binarios usa el patrón `fread(); while(!feof()) { ...; fread(); }`.
- Los archivos binarios tienen registros de tamaño fijo para permitir acceso directo con `ftell` y `sizeof`.
