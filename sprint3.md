# Sprint 3: Cwitter - Hilos, Escalabilidad y Rendimiento

Este archivo detalla los pasos estructurados, requerimientos, casos de prueba y entregables para el tercer sprint (final) del proyecto Cwitter, basado en las especificaciones del cliente y los criterios de evaluación de la cátedra de Algoritmos y Estructuras de Datos (UNLaM).

---

## Paso 1: Planificación, Análisis y Medición de Rendimiento (Bitácora de Diseño)

* [x] **1.1. Benchmark Inicial de Búsqueda:** Implementar un módulo o script de medición para evaluar el algoritmo de búsqueda secuencial actual sobre la estructura de tweets. Medir cantidad de comparaciones y tiempo de ejecución en el peor caso con volúmenes de 100, 1.000, 10.000 y 100.000 tweets.
* [x] **1.2. Análisis de Escalabilidad:** Analizar si el tiempo de búsqueda crece de forma lineal $O(N)$ y evaluar la necesidad de incorporar una estructura de búsqueda eficiente (como un Árbol Binario de Búsqueda / AVL o Tabla Hash / Indexación) para optimizar las consultas a $O(\log N)$ o $O(1)$.
* [x] **1.3. Selección de Estructura para Hilos de Conversación:** Diseñar y seleccionar la estructura de datos para representar respuestas anidadas sin límite de profundidad. Evaluar alternativas (Árbol N-ario con representación *Primer Hijo - Siguiente Hermano*, punteros jerárquicos padre-hijo, o listas de adyacencia) y justificar la decisión técnica.
* [x] **1.4. Rediseño del Formato de Persistencia:** Definir el nuevo esquema de almacenamiento en `tweets.txt` para persistir la jerarquía de respuestas (ej. incorporación del campo `id_padre`, donde `-1` o `0` representa un tweet raíz).
* [x] **1.5. Documentación en `BITACORA.md`:** 
    * Registrar la tabla de cantidad de tweets vs. tiempo/comparaciones de búsqueda (antes y después de la optimización).
    * Justificar la estructura elegida para los hilos frente a otras alternativas conocidas.
    * Explicar el cambio en el formato de persistencia para mantener la relación padre-hijo al reiniciar la aplicación.
    * Registrar al menos 2 interacciones con IA (prompts, resúmenes, evaluación y decisiones) y al menos un caso de respuesta incorrecta/subóptima corregida.
    * Registrar el planteo inicial y dudas de cada integrante.

---

## Paso 2: Diseño de Casos de Prueba (Testing)

* [x] **2.1. Actualización de `TESTS.md`:** Diseñar y documentar al menos 6 casos de prueba significativos antes de comenzar la implementación de código, orientados a escenarios críticos y límites de la nueva funcionalidad.
* [x] **2.2. Caso 1 (Límite de caracteres ampliado):** Publicación de un tweet con hasta 280 caracteres y verificación del rechazo ante textos que superen dicho límite (ej. 281 caracteres).
* [x] **2.3. Caso 2 (Respuesta de primer nivel):** Publicación de una respuesta directa a un tweet existente (hilo con profundidad 1) y verificación del enlace al tweet padre.
* [x] **2.4. Caso 3 (Respuestas anidadas multinivel):** Publicación de respuestas encadenadas (respuesta a una respuesta) validando que la estructura soporte profundidad arbitraria (profundidades 2, 3 o más).
* [x] **2.5. Caso 4 (Respuesta a tweet inexistente o inválido):** Intento de responder a un ID de tweet inexistente, verificando el rechazo de la operación y la integridad del sistema.
* [x] **2.6. Caso 5 (Visualización jerárquica del hilo):** Visualización del árbol de respuestas completo a partir de un tweet raíz, comprobando la correcta distinción visual (sangría / niveles) entre tweets padres e hijos.
* [x] **2.7. Caso 6 (Persistencia y reconstrucción de hilos):** Guardado, cierre y reinicio del programa, validando que la jerarquía de hilos y sus respuestas se reconstruyan idénticamente en memoria sin pérdidas ni ciclos.
* [x] **2.8. Casos Adicionales de Estrés y Borde:** Incorporar casos para eliminación de un tweet con respuestas asociadas (borrado en cascada o marcación) y comportamiento ante gran volumen de inserciones simultáneas.

---

## Paso 3: Desarrollo y Nuevas Funcionalidades

* [x] **3.1. Ampliación del límite de contenido a 280 caracteres:**
    * Modificar la constante `MAX_TWEET` de 140 a 280 en `include/cwitter.h`.
    * Ajustar buffers de lectura, validaciones y formato de visualización en `src/cwitter.c` y `main.c`.
* [x] **3.2. Adaptación de la Entidad Tweet y Estructuras de Hilos:**
    * Incorporar el campo `id_padre` en la estructura `Tweet` para identificar a qué publicación responde (`-1` para tweets originales).
    * Definir los tipos de datos y punteros para la jerarquía de respuestas (árbol n-ario mediante lista de hijos o enlace primer hijo / hermano siguiente).
* [x] **3.3. Funcionalidad de Respuesta a Tweets:**
    * Implementar la función `responderTweet(...)` permitiendo asociar un nuevo tweet a uno existente por su ID.
    * Validar que el tweet padre exista antes de registrar la respuesta.
    * Actualizar el menú de usuario en `main.c` para ofrecer la opción de responder a un tweet seleccionado.
* [x] **3.4. Visualización del Hilo Completo de Conversación:**
    * Implementar el recorrido en profundidad (DFS / Pre-order recursivo) para imprimir un tweet y todas sus respuestas de manera indentada (ej. usando prefijos como `└── [ID: X] @usuario: texto`).
    * Distinguir claramente los niveles de profundidad para que el usuario identifique la línea de conversación.
* [x] **3.5. Optimización del Motor de Búsqueda por ID:**
    * Implementar la estructura seleccionada en el Paso 1.2 (árbol binario de búsqueda o tabla de índices) para indexar los tweets por `id`.
    * Sustituir la búsqueda lineal en operaciones críticas por la búsqueda optimizada para garantizar escalabilidad frente a 10.000+ tweets.
* [x] **3.6. Persistencia Jerárquica:**
    * Modificar `guardarTweets(...)` para almacenar el campo `id_padre` en `tweets.txt` (`id;id_padre;autor;contenido`).
    * Modificar `cargarTweets(...)` garantizando que al leer los registros se reconstruya fielmente la estructura en memoria, contemplando casos donde las respuestas se carguen tras su padre correspondiente.
* [x] **3.7. Interfaz de Usuario y Menú Principal:**
    * Agregar al menú de usuario opciones para:
        * Ver Hilo Completo de un Tweet.
        * Responder a un Tweet existente.
        * Ejecutar Benchmark / Test de Rendimiento de Búsqueda.
* [x] **3.8. Verificación de Integridad con Funcionalidades Anteriores:**
    * Asegurar que el soporte de FameCheck, verificación de usuarios, logs de errores, Ranking Top 5 y navegación del feed (Sprint 2) sigan funcionando sin regresiones.

---

## Paso 4: Validación, Benchmark y Calidad de Código

* [x] **4.1. Ejecución y Reporte del Benchmark:**
    * Correr las pruebas con 100, 1.000, 10.000 y 100.000 tweets.
    * Tabular los tiempos reales y cantidad de comparaciones, volcando los datos definitivos en `BITACORA.md`.
* [x] **4.2. Ejecución de la Suite de Pruebas:**
    * Ejecutar los 6 casos de prueba obligatorios de Sprint 3 y los casos de regresión de Sprints 1 y 2.
    * Completar en `TESTS.md` la columna de "Salida obtenida" para cada prueba.
* [x] **4.3. Verificación de Memoria y Calidad ANSI C:**
    * Verificar la ausencia de fugas de memoria (`memory leaks`), especialmente al liberar las ramas y nodos del árbol de respuestas (`liberarHilo(...)`).
    * Compilar con flags estrictas (`-Wall -Wextra -pedantic -std=c99` o `c89`) garantizando 0 errores y 0 warnings.
    * Verificar que ningún archivo quede abierto ni existan accesos indebidos a memoria.

---

## Paso 5: Entrega del Sprint 3 (Entrega Final)

* [x] **5.1. Limpieza del Entorno:** Eliminar ejecutables temporales, archivos de logs de prueba y carpetas de compilación (`bin/`, `obj/`).
* [x] **5.2. Rama de Release:** Posicionarse en la rama `release/v3-final` y fusionar los cambios validados.
* [x] **5.3. Etiquetado Git:** Crear el tag definitivo `v3-final` sobre el último commit de la entrega.
* [ ] **5.4. Empaquetado:** Comprimir el proyecto con la nomenclatura requerida: `NOMBREDEGRUPO_TP_SPRINT3.zip` (o entrega final según pautas).
* [ ] **5.5. Entrega en MIeL:** Cargar el archivo `.zip` en la plataforma MIeL adjuntando en el texto de entrega el enlace al repositorio privado de GitHub con los colaboradores correspondientes.
