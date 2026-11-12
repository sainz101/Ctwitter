# Cwitter - Trabajo Práctico de Algoritmos y Estructuras de Datos

**Cátedra:** Algoritmos y Estructuras de Datos (AyED 3640)  
**Institución:** Centro Regional Universitario de Ituzaingó (CRUI / UNLaM) - Comisión 90-3900  
**Versión de Entrega:** Sprint 3 (Entrega Final - Tag `v3-final`)

---

## 🚀 Descripción del Proyecto
Cwitter es una plataforma de red social por consola desarrollada en **ANSI C (C99)** que integra múltiples estructuras dinámicas de datos para resolver requerimientos de escalabilidad, navegación interactiva y jerarquía de publicaciones:
1. **Núcleo Global:** TAD Lista Simplemente Enlazada con punteros `cabeza` y `ultimo` ($O(1)$ inserción al final).
2. **Hilos de Conversación:** TAD Árbol N-ario representado mediante punteros *Primer Hijo - Siguiente Hermano* (*Left-Child / Right-Sibling*), soportando profundidad infinita de respuestas.
3. **Motor de Búsqueda Escalable:** Índice basado en **Árbol Binario de Búsqueda (ABB)** que reduce la búsqueda por ID de $O(N)$ a $O(\log N)$ para soportar más de 100.000 publicaciones.
4. **Feed Interactivo (Modo Historia):** TAD Lista Doblemente Enlazada para navegación bidireccional ($O(1)$).
5. **Ranking Top 5:** TAD Lista Enlazada Simple con Inserción Ordenada.
6. **Integración Externa:** Simulación y cliente FameCheck con política de tolerancia a fallos (*Fail-Safe*) y archivo acumulativo de logs `service_errors.log`.

---

## 📁 Estructura del Repositorio
```text
.
├── include/
│   └── cwitter.h               # Prototipos y TADs con documentación técnica Big-O
├── src/
│   └── cwitter.c               # Implementación de algoritmos, TADs y gestión de memoria
├── main.c                      # Punto de entrada y menú interactivo de usuario
├── benchmark_busqueda.c        # Módulo de análisis empírico y benchmark de escalabilidad
├── BITACORA.md                 # Registro de decisiones de diseño, benchmark y uso de IA
├── TESTS.md                    # Plan de 30 casos de prueba con comprobaciones empíricas
├── Guia_de_funcionamiento.md   # Guía técnica de funcionamiento, decisiones de diseño y análisis Big-O
├── TP-AyE_de_Datos/            # Proyecto y workspace de Code::Blocks
├── AGENT/                      # Consignas oficiales, enunciados de sprints y reglamentos
└── README.md                   # Documentación general del repositorio
```

---

## 🛠️ Instrucciones de Compilación y Ejecución

### Opción 1: Compilación por Consola (GCC)
El proyecto cumple estrictamente el estándar ANSI C / C99 y compila con **0 advertencias y 0 errores** bajo directivas estrictas:
```bash
gcc -Wall -Wextra -pedantic -std=c99 -Iinclude main.c src/cwitter.c -o cwitter.exe
./cwitter.exe
```

### Opción 2: Módulo Standalone de Benchmark
```bash
gcc -Wall -Wextra -pedantic -std=c99 benchmark_busqueda.c -DBENCHMARK_STANDALONE -o bench.exe
./bench.exe
```

### Opción 3: Entorno Code::Blocks
Abrir el archivo de proyecto `TP-AyE_de_Datos/TP-AyE_de_Datos.cbp` y presionar **Build and Run** (F9).

---

## 📚 Documentación para la Evaluación y Defensa Oral
* **Bitácora de Diseño:** Ver [BITACORA.md](file:///C:/Users/matuc/Desktop/TP de estructuras de datos/BITACORA.md).
* **Casos de Prueba:** Ver [TESTS.md](file:///C:/Users/matuc/Desktop/TP de estructuras de datos/TESTS.md).
* **Guía de Funcionamiento y Diseño:** Ver [Guia_de_funcionamiento.md](file:///C:/Users/matuc/Desktop/TP de estructuras de datos/Guia_de_funcionamiento.md).
