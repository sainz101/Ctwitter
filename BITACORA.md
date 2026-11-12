# Bitácora de Diseño - Cwitter (Sprints 1 y 2)

Este documento registra las decisiones arquitectónicas, alternativas consideradas, justificaciones y el uso de Inteligencia Artificial para el desarrollo de Cwitter.

---

## SPRINT 1: Fundamentos (Colas)

### 1. Identificación de Entidades
Para el Sprint 1 se definieron las entidades `Usuario` (nombre y clave) y `Tweet` (id, autor, contenido).

### 2. Estructuras de Datos y Justificación
Siguiendo la directiva de **prohibición de listas**, se seleccionó la estructura de **Cola Dinámica (Queue)** para usuarios y tweets.
*   **Justificación:** Garantiza un acceso ordenado FIFO y cumple con la restricción normativa de no usar listas genéricas.

---

## SPRINT 2: Procesamiento y Verificación

En este sprint, evolucionamos la arquitectura para soportar navegación interactiva y servicios externos, incorporando nuevas estructuras de datos permitidas.

### 1. Nuevas Estructuras de Datos y Justificación

#### A. Navegación del Feed: Lista Doblemente Enlazada
*   **Decisión:** Para el modo de visualización "Historia" (un tweet a la vez), hemos implementado una **Lista Doblemente Enlazada**.
*   **Justificación:** El requerimiento de "volver al anterior" de forma fluida hace que la Cola (FIFO) del Sprint 1 sea insuficiente. Una lista doble permite punteros `siguiente` y `anterior`, logrando una navegación bidireccional en tiempo constante $O(1)$ entre nodos una vez generada la vista.
*   **Alternativa descartada:** Uso de dos Pilas. Aunque funcional, la gestión de memoria y el traspaso de elementos entre pilas resulta más costoso y menos intuitivo para una navegación repetitiva.

#### B. Ranking Top 5: Lista Simple Ordenada
*   **Decisión:** Para el ranking de usuarios verificados, utilizamos una **Lista Enlazada Simple con Inserción Ordenada**.
*   **Justificación:** Dado que los datos de publicaciones llegan desordenados desde el servicio externo, insertar cada usuario en su posición correcta según su contador de tweets mantiene el ranking actualizado en todo momento. Al finalizar el proceso, solo se toman los primeros 5 nodos.

### 2. Integración con Servicio Externo (FameCheck)

#### A. Verificación y Persistencia
*   Se añade el campo `es_verificado` a la entidad `Usuario`. 
*   **Decisión:** La consulta al servicio se realiza durante el login o registro. El estado se persiste en el archivo `usuarios.txt` para evitar consultas innecesarias en cada inicio de sesión, respetando la autoridad del servicio externo.

#### B. Tolerancia a Fallos y Logging
*   **Estrategia:** "Fail-safe". Si el reporte de un tweet a FameCheck falla, la publicación prosigue para no afectar la experiencia del usuario.
*   **Implementación de Logs:** Se crea el archivo `service_errors.log` donde se registran los fallos con marca temporal. Se eligió un formato de texto plano (Append) por su simplicidad y bajo costo de escritura.

### 3. Algoritmo de Ordenamiento del Feed

*   **Lógica:** Priorización por tipo de cuenta.
*   **Algoritmo:** Al generar la vista del feed, se realiza un filtrado en dos pasadas sobre la Cola Global de Tweets:
    1.  Pasada 1: Extraer tweets de autores con `es_verificado == 1`.
    2.  Pasada 2: Extraer tweets de autores con `es_verificado == 0`.
*   Esto garantiza que los verificados aparezcan primero, manteniendo el orden cronológico dentro de cada grupo.

---

## Registro de Interacciones con la IA (Sprint 2)

### Interacción 1 (Planificación de Estructuras)
*   **Prompt:** "¿Cuál es la mejor estructura para un feed que permite ir hacia adelante y atrás?"
*   **Resumen:** La IA recomendó la Lista Doblemente Enlazada por su capacidad natural de navegación bidireccional.
*   **Decisión:** Se adoptó esta estructura para el modo interactivo del feed.

### Interacción 2 (Gestión de Errores)
*   **Prompt:** "¿Cómo registrar errores de red en C sin detener la ejecución principal?"
*   **Resumen:** La IA sugirió el uso de funciones de logging que escriban en archivos en modo "append" (`a`) y el manejo de códigos de retorno para decidir si mostrar un mensaje al usuario o solo loguear el error de forma silenciosa.
*   **Decisión:** Implementación de `registrarErrorServicio()` para el archivo de logs.


---

## REFACTORIZACIÓN: De Cola a Lista Enlazada

### 1. Decisión Arquitectónica y Justificación
* **Problema:** En el Sprint 1 se utilizó una Cola por imposición de las reglas iniciales. Sin embargo, operaciones como búsqueda arbitraria, eliminación intermedia y ordenamientos violan el principio FIFO de una Cola.
* **Alternativas evaluadas:**
  1. *Mantener Cola Dinámica:* Requiere romper el TAD para acceder a nodos intermedios, resultando en un código conceptualmente incorrecto.
  2. *Migrar a Vector Dinámico:* Presenta alto costo de desplazamiento en inserciones/eliminaciones intermedias ($O(N)$) y riesgo de fragmentación de memoria.
  3. *Migrar a TAD Lista Enlazada con punteros Cabeza y Último:* Permite inserción al final en $O(1)$, eliminación y búsqueda prolijas sin violentar disciplinas de acceso, y sienta la base para enlazar las listas de respuestas en el Sprint 3.
* **Elección y Justificación:** Se eligió la Lista Enlazada. Devuelve la coherencia formal a los TADs del sistema, respeta la encapsulación y simplifica la incorporación de hilos jerárquicos.
* **Impacto en persistencia:** Ninguno; los archivos `usuarios.txt` y `tweets.txt` mantienen total compatibilidad.


---

## SPRINT 3: Hilos, Escalabilidad y Rendimiento

### 1. Benchmark de Rendimiento y Análisis de Escalabilidad

Para validar empíricamente la necesidad de optimizar el motor de búsqueda por ID ante el crecimiento exponencial de la plataforma, se diseñó un módulo de prueba (`benchmark_busqueda.c`) midiendo la cantidad de comparaciones y el tiempo de respuesta en el **peor caso** (búsqueda de un tweet inexistente) para volúmenes de 100, 1.000, 10.000 y 100.000 tweets.

#### Tabla Comparativa de Rendimiento

| Cantidad de Tweets ($N$) | Comparaciones (Lineal) | Tiempo Lineal ($ms$) | Comparaciones (ABB Optimizado) | Tiempo ABB ($ms$) |
| :---: | :---: | :---: | :---: | :---: |
| **100** | 100 | 0.00010 ms | **7** | < 0.00001 ms |
| **1.000** | 1.000 | 0.00100 ms | **10** | 0.00002 ms |
| **10.000** | 10.000 | 0.02300 ms | **14** | < 0.00001 ms |
| **100.000** | 100.000 | 0.27000 ms | **17** | 0.00002 ms |

#### Conclusión del Análisis:
* **Búsqueda Lineal:** Presenta una complejidad temporal $O(N)$ donde tanto el tiempo como la cantidad de operaciones crecen en proporción directa al volumen de tweets. Para $100.000$ tweets, requiere $100.000$ comparaciones por búsqueda, lo cual degrada de manera acumulativa las operaciones repetitivas (como la reconstrucción del árbol de hilos o la búsqueda de tweets padre).
* **Búsqueda Optimizada con ABB:** Presenta una complejidad temporal logarítmica $O(\log_2 N)$. Para $100.000$ tweets, el peor caso se resuelve en tan solo **17 comparaciones**, logrando una reducción del **99.98%** en ciclos de CPU. Por lo tanto, se incorpora un índice basado en **Árbol Binario de Búsqueda** para la resolución eficiente de búsquedas por ID.

---

### 2. Estructura de Datos para Hilos de Conversación (Árbol N-ario)

* **Problema:** Un tweet puede recibir respuestas, y esas respuestas pueden tener las suyas propias, armando conversaciones de profundidad arbitraria sin límite predefinido.
* **Alternativas evaluadas:**
  1. *Vector estático de punteros a hijos por tweet:* Descartado inmediatamente por violar la regla de no fijar capacidades arbitrarias e imponer un límite artificial al número de respuestas.
  2. *Lista enlazada de punteros a hijos dentro de cada nodo:* Es funcional, pero requiere la reserva dinámica de dos tipos distintos de nodos (nodo tweet y nodo lista-hijo), complejizando el manejo de memoria y aumentando el riesgo de fragmentación.
  3. *Árbol N-ario con representación "Primer Hijo - Siguiente Hermano" (Left-Child, Right-Sibling):* Cada nodo de tweet posee dos punteros jerárquicos: `primer_hijo` (puntero a la primera respuesta directa) y `siguiente_hermano` (puntero a la siguiente respuesta al mismo nivel del mismo tweet padre).
* **Elección y Justificación:** Se eligió la representación **Primer Hijo - Siguiente Hermano**. Esta arquitectura de Árbol N-ario permite:
  * Profundidad ilimitada de conversación ($N$ niveles).
  * Cantidad ilimitada de respuestas por tweet ($M$ hermanos).
  * Consumo mínimo de memoria: exactamente 2 punteros por nodo sin estructuras auxiliares intermedias.
  * Recorrido natural en profundidad (DFS / Pre-order recursivo) para renderizar visualmente el hilo con sangría según el nivel.

---

### 3. Rediseño del Formato de Persistencia Jerárquica

* **Problema:** En los Sprints 1 y 2, cada tweet era un registro plano sin dependencias. Con la incorporación de hilos, la relación padre-hijo debe sobrevivir al cierre y reinicio del sistema.
* **Solución Implementada:**
  * Se rediseñó el archivo `tweets.txt` incorporando el campo `id_padre` con formato delimitado por punto y coma:
    ```text
    id;id_padre;autor;contenido
    ```
  * Convención:
    * `id_padre = -1`: Representa un tweet original (raíz del hilo).
    * `id_padre >= 0`: Representa una respuesta directa al tweet cuyo identificador es `id_padre`.
  * **Reconstrucción en memoria:** Al iniciar Cwitter, `cargarTweets` lee los registros, crea los nodos y vincula cada respuesta como hijo/hermano de su tweet padre mediante la búsqueda eficiente en el índice ABB. Se mantiene retrocompatibilidad con archivos de sprints anteriores que no poseían el campo `id_padre`.

---

### 4. Registro de Interacciones con la IA (Sprint 3)

#### Interacción 1 (Modelado de Árboles N-arios en C)
* **Prompt:** "¿Cómo representar en ANSI C un hilo de respuestas de tweets con profundidad infinita y ramificación ilimitada sin usar arrays de tamaño fijo?"
* **Resumen:** La IA propuso la técnica de Árbol N-ario mediante punteros *Primer Hijo - Siguiente Hermano*, explicando cómo mapear cualquier árbol general a una estructura binaria conceptual de punteros.
* **Evaluación y Decisión:** Se adoptó esta estructura porque evita vectores auxiliares, cumple estrictamente las normas de eficiencia y permite un recorrido recursivo simple para dibujar el árbol con sangrías.

#### Interacción 2 (Detección de Error / Solución Subóptima de la IA)
* **Prompt:** "¿Cómo reconstruir los hilos de tweets al leerlos desde el archivo de texto?"
* **Respuesta de la IA (Subóptima/Errónea):** La IA propuso realizar un recorrido cuadrático $O(N^2)$: por cada tweet leído, recorrer secuencialmente toda la lista enlazada para buscar al padre y enlazarlo.
* **Por qué era subóptima:** Si un archivo contiene $10.000$ tweets, un algoritmo $O(N^2)$ ejecutaría hasta $100.000.000$ de operaciones en el arranque del sistema, congelando la aplicación y contradiciendo el objetivo de escalabilidad de este sprint.
* **Solución adoptada por el equipo:** Utilizamos el índice **Árbol Binario de Búsqueda (ABB)** construido al vuelo durante la carga. Al leer cada tweet, la búsqueda de su padre se resuelve en tiempo $O(\log N)$, reduciendo el tiempo total de carga a $O(N \log N)$ y garantizando un arranque instantáneo.

---

### 5. Planteo Inicial y Dudas de los Integrantes (Sprint 3)
* **Integrante 1:** Planteó la duda sobre cómo diferenciar un tweet original de una respuesta al mostrar el feed global para no saturarlo con respuestas fuera de contexto. Se decidió que el feed principal priorice tweets originales y ofrezca la opción de desplegar el hilo completo a demanda.
* **Integrante 2:** Analizó el riesgo de fuga de memoria al eliminar un tweet que tiene un árbol entero de respuestas. Se determinó implementar una función de liberación recursiva de subárboles para garantizar 0 pérdidas de memoria.
* **Integrante 3:** Evaluó el impacto del aumento a 280 caracteres en los buffers de entrada de `scanf` y `fgets`, diseñando un mecanismo de limpieza de buffer (`stdin`) para evitar saltos involuntarios en los menús.
