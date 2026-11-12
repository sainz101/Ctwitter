# Guía de Funcionamiento y Decisiones Arquitectónicas - Cwitter
**Materia:** Algoritmos y Estructuras de Datos (AyED 3640)  
**Institución:** Centro Regional Universitario de Ituzaingó (CRUI / UNLaM) - Comisión 90-3900  
**Proyecto:** Cwitter - Red Social Académica  

---

## 1. Resumen de la Evolución del Proyecto (Los 3 Sprints)

| Sprint | Requerimiento del Cliente | Estructuras Implementadas | Justificación Técnica |
| :--- | :--- | :--- | :--- |
| **Sprint 1: Fundamentos** | Registro de usuarios, publicación (máx 140 car.), modificación, eliminación, feed y búsqueda. | **Cola Dinámica (Queue)** para usuarios y tweets. | Impuesto por la consigna inicial (prohibición de listas). Garantizaba orden cronológico FIFO de llegada. |
| **Sprint 2: Procesamiento** | Verificación externa (FameCheck), Top 5 usuarios activos, feed con prioridad a verificados, navegación interactiva (Historias). | - **Lista Doblemente Enlazada** (Modo Historia).<br>- **Lista Simple con Inserción Ordenada** (Top 5).<br>- Logging Fail-Safe (`service_errors.log`). | - Navegación adelante/atrás en $O(1)$.<br>- Ranking mantenido en orden dinámico descendente.<br>- Tolerancia a fallos: caídas de red no cancelan la publicación. |
| **Sprint 3: Hilos y Escalabilidad (Final)** | 280 caracteres, respuestas anidadas sin límite de profundidad, escalabilidad para 100.000 tweets, persistencia de hilos. | - **Árbol N-ario** (Primer Hijo - Siguiente Hermano).<br>- **Árbol Binario de Búsqueda (ABB)** como índice.<br>- **Lista Enlazada** (Refactorización definitiva del núcleo). | - Hilos con ramificación y niveles infinitos sin vectores fijos.<br>- Búsqueda reducida de $O(N)$ a $O(\log N)$ (de 100.000 a 17 comparaciones).<br>- Reconstrucción de persistencia en $O(N \log N)$ en lugar de $O(N^2)$. |

---

## 2. Cuadro Comparativo de Complejidades (Big-O)

| Operación | Estructura de Datos | Complejidad Temporal | Complejidad Espacial | Justificación |
| :--- | :--- | :---: | :---: | :--- |
| **Insertar Tweet al final** | Lista Enlazada (`ultimo`) + ABB | $O(\log N)$ | $O(1)$ nuevo nodo | Inserción en lista es $O(1)$; indexación en ABB es $O(\log N)$. |
| **Buscar Tweet por ID** | Árbol Binario de Búsqueda (ABB) | $O(\log_2 N)$ | $O(1)$ | En cada paso descarta la mitad de las claves restantes. |
| **Buscar Tweet por ID (Original)** | Búsqueda Secuencial en Lista/Cola | $O(N)$ | $O(1)$ | En el peor caso recorre los $N$ nodos secuencialmente. |
| **Responder a un Tweet** | ABB + Árbol N-ario | $O(\log N) + O(H)$ | $O(1)$ nuevo nodo | Localiza al padre en $O(\log N)$ y lo enlaza al final de sus hermanos. |
| **Renderizar Hilo Completo** | Árbol N-ario (DFS Pre-Order) | $O(K)$ | $O(P)$ pila llamadas | $K$ = tweets del hilo; $P$ = profundidad del subárbol. |
| **Navegar en Modo Historia** | Lista Doblemente Enlazada | $O(1)$ por salto | $O(1)$ | Salto directo por puntero `siguiente` o `anterior`. |
| **Insertar en Ranking Top 5** | Lista Simple Ordenada | $O(U)$ | $O(1)$ nuevo nodo | Recorre hasta encontrar la posición según cantidad de tweets. |
| **Carga de Persistencia (tweets.txt)** | Lista + ABB (2 Fases) | $O(N \log N)$ | $O(N)$ en memoria | Fase 1 carga e indexa ($N \log N$); Fase 2 vincula hilos ($N \log N$). |
| **Liberación Total de Memoria** | Recorrido secuencial + ABB | $O(N)$ | $O(1)$ | Se liberan todos los nodos dinámicos sin memory leaks. |

---

## 3. Preguntas Frecuentes, Fundamentos de Diseño y Defensa Técnica

### Pregunta 1: ¿Por qué en el Sprint 1 usaron una Cola y luego en el Sprint 3 pasaron a una Lista Enlazada?
> **Respuesta Modelo:**
> "En el Sprint 1 la consigna prohibía explícitamente el uso de listas, por lo que adoptamos una Cola Dinámica para garantizar el principio FIFO (First-In, First-Out) de las publicaciones cronológicas.  
> Sin embargo, una red social real exige búsquedas intermedias arbitrarias, modificaciones y eliminaciones de tweets específicos. En una Cola pura, estas operaciones violan el principio de abstracción, ya que solo se debería poder acceder al `frente` (desencolar) o al `final` (encolar). Para el Sprint 3, la cátedra habilitó la refactorización a **Lista Simplemente Enlazada con punteros a Cabeza y Último**: esto recuperó la coherencia teórica de los TADs, mantuvo la inserción al final en tiempo constante $O(1)$, y facilitó la integración limpia del árbol de respuestas y el índice de búsqueda."

---

### Pregunta 2: ¿Cómo modelaron los hilos de respuestas sin límite de profundidad? ¿Qué alternativas evaluaron y por qué las descartaron?
> **Respuesta Modelo:**
> "Modelamos las conversaciones como un **Árbol N-ario** utilizando la representación estándar de **Primer Hijo - Siguiente Hermano** (*Left-Child / Right-Sibling*):
> - `primer_hijo`: Apunta a la primera respuesta directa que recibió la publicación.
> - `siguiente_hermano`: Apunta a la siguiente respuesta del mismo nivel bajo el mismo padre.
> 
> **Alternativas descartadas:**
> 1. *Vector estático de punteros a hijos:* Violaba la regla de no poner capacidades fijas y limitaba artificialmente la cantidad de respuestas.
> 2. *Lista enlazada de punteros hijos dentro de cada nodo:* Funcionaba, pero requería crear un tipo de nodo extra (nodo de lista de adyacencia), duplicando asignaciones de memoria y aumentando el riesgo de fragmentación.
> 
> La solución de *Primer Hijo - Siguiente Hermano* requiere únicamente 2 punteros por nodo, soporta profundidad infinita y ramificación ilimitada con el consumo mínimo de memoria."

---

### Pregunta 3: ¿Cómo funciona el algoritmo para mostrar el hilo completo de un tweet?
> **Respuesta Modelo:**
> "Utilizamos un recorrido en profundidad **DFS (Depth-First Search) en Pre-Order recursivo** implementado en `imprimirSubarbolHilo`:
> 1. **Caso base:** Si el nodo es `NULL`, la función retorna.
> 2. **Procesamiento:** Imprime el tweet actual con una sangría proporcional al parámetro `nivel` (`nivel * 4 espacios` y caracteres conectores `└── `).
> 3. **Recursión:** Recorre la lista de hermanos a partir de `nodo->primer_hijo`, llamando recursivamente a la función con `nivel + 1`.
> 
> Complejidad temporal: $O(K)$, donde $K$ es el número de tweets dentro de la conversación.  
> Complejidad espacial: $O(H)$ en la pila de llamadas (*call stack*), donde $H$ es la altura máxima del hilo."

---

### Pregunta 4: ¿Por qué incorporaron un Árbol Binario de Búsqueda (ABB) y qué demostró el Benchmark?
> **Respuesta Modelo:**
> "En el Sprint 3 el cliente proyectó un crecimiento de hasta 100.000 tweets. Al medir la búsqueda lineal original sobre la lista enlazada en el peor caso (buscar un ID inexistente $N+1$), comprobamos que:
> - Para $100$ tweets: $100$ comparaciones.
> - Para $10.000$ tweets: $10.000$ comparaciones.
> - Para $100.000$ tweets: $100.000$ comparaciones ($0.35$ ms por consulta).
> 
> El tiempo y las operaciones crecían linealmente $O(N)$, lo cual volvía prohibitiva la reconstrucción de miles de hilos al iniciar el programa.  
> Al incorporar un **Índice ABB** (`NodoIndiceABB`), la búsqueda por ID se redujo a tiempo logarítmico $O(\log_2 N)$. Para $100.000$ tweets, el ABB resuelve la búsqueda en **solo 17 comparaciones**, logrando un ahorro del **99.98%** en ciclos de CPU."

---

### Pregunta 5: ¿Cómo resolvieron la persistencia jerárquica de los hilos en `tweets.txt`?
> **Respuesta Modelo:**
> "Modificamos el formato plano del archivo añadiendo el campo `id_padre`:
> ```text
> id;id_padre;autor;contenido
> ```
> - Si `id_padre == -1`: Es un tweet original / raíz.
> - Si `id_padre >= 0`: Es una respuesta al tweet con dicho ID.
> 
> **Reconstrucción eficiente en 2 Fases:**
> Si al leer el archivo buscáramos al padre secuencialmente en la lista, el arranque sería cuadrático $O(N^2)$ (error que detectamos en la propuesta inicial de la IA). En su lugar hicimos:
> - **Fase 1:** Leer secuencialmente `tweets.txt`, crear los nodos en la lista e indexarlos inmediatamente en el ABB ($O(N \log N)$).
> - **Fase 2:** Recorrer los tweets y, para cada uno con `id_padre >= 0`, ubicar a su padre instantáneamente mediante el ABB en $O(\log N)$ y enlazarlo en el árbol ($O(N \log N)$).  
> Esto garantiza retrocompatibilidad y arranque en milisegundos."

---

### Pregunta 6: ¿Qué ocurre al eliminar un tweet que tiene respuestas? ¿Cómo evitan punteros colgantes (dangling pointers)?
> **Respuesta Modelo:**
> "En `eliminarTweet` implementamos una estrategia estricta de integridad de punteros y memoria (Caso de Prueba CP-30):
> 1. **Borrado en cascada recursivo:** Mediante `eliminarRespuestasRecursivo`, se recorre el subárbol de respuestas liberando dinámicamente cada nodo hijo y removiéndolo de la lista global secuencial.
> 2. **Desvinculación:** Si el tweet a eliminar era respuesta de otro, `desvincularDeArbolHilos` lo desconecta del `primer_hijo` o del `siguiente_hermano` correspondiente.
> 3. **Extracción de la lista:** Se actualizan los punteros `siguiente`, `cabeza` y `ultimo`.
> 4. **Liberación:** Se ejecuta `free(actual)`.
> 5. **Actualización del Índice ABB:** Se reconstruye el índice ABB sobre los nodos restantes para garantizar que no quede ningún puntero apuntando a memoria liberada (*dangling pointer*)."

---

### Pregunta 7: ¿Por qué para el Modo Historia del Sprint 2 usaron una Lista Doblemente Enlazada y no dos Pilas?
> **Respuesta Modelo:**
> "El requerimiento pedía recorrer el feed publicación por publicación, avanzando y retrocediendo como en historias de Instagram.
> - Con una **Lista Doblemente Enlazada** (`NodoTweetDoble` con punteros `siguiente` y `anterior`), avanzar es `actual = actual->siguiente` en $O(1)$ y retroceder es `actual = actual->anterior` en $O(1)$.
> - Evaluamos la alternativa de usar **dos Pilas** (una pila para tweets anteriores y otra para futuros). Si bien permite ir y volver, requiere operaciones `pop` y `push` constantes con trasvase continuo de elementos y copias de memoria en cada cambio de sentido.
> - La lista doblemente enlazada es la estructura canónica, más limpia y con cero sobrecosto de transferencia."

---

### Pregunta 8: ¿Cómo funciona el Ranking Top 5 de usuarios más activos del Sprint 2?
> **Respuesta Modelo:**
> "Utilizamos una **Lista Enlazada Simple con Inserción Ordenada** (`NodoRanking`):
> 1. Se recorre la lista de usuarios filtrando aquellos con `es_verificado == 1`.
> 2. Para cada usuario verificado, se contabilizan sus tweets en la lista global.
> 3. Se inserta en la lista de ranking buscando su posición de modo que la lista siempre quede ordenada de mayor a menor cantidad de tweets.
> 4. Al finalizar, simplemente se leen los primeros 5 nodos.
> Esto evita tener que ordenar un vector estático con algoritmos pesados y no impone límites fijos."

---

### Pregunta 9: ¿Cómo garantizaron la ausencia total de fugas de memoria (memory leaks)?
> **Respuesta Modelo:**
> "Aplicamos el principio de que todo `malloc` debe tener su correspondiente `free`:
> 1. **Lista de Usuarios:** `liberarListaUsuarios` recorre la lista secuencial liberando cada `NodoUsuario`.
> 2. **Lista de Tweets y Árbol:** Como todos los tweets (raíces y respuestas) pertenecen a la lista secuencial global `cabeza -> siguiente`, liberar la lista secuencial libera cada nodo exactamente una sola vez (evitando *double-free* y *memory leaks*).
> 3. **Índice ABB:** `liberarIndiceABB` recorre el árbol en Post-Order (subárbol izquierdo, subárbol derecho, y raíz) liberando cada `NodoIndiceABB`.
> 4. **Estructuras auxiliares temporales:** Tanto la lista doble del Modo Historia (`liberarFeedInteractivo`) como la lista del ranking (`liberarRanking`) se liberan inmediatamente después de usarse.
> 5. **Archivos:** Todos los `fopen` cuentan con su respectivo `fclose` en todas las rutas de ejecución."

---

### Pregunta 10: ¿Cómo manejaron la integración con FameCheck y qué significa Fail-Safe?
> **Respuesta Modelo:**
> "FameCheck es un servicio externo que determina qué cuentas son figuras públicas.
> - **Persistencia de estado:** El resultado de la verificación se guarda en el archivo `usuarios.txt` para no consultar a la API innecesariamente en cada inicio de sesión.
> - **Estrategia Fail-Safe:** Cuando un usuario verificado publica y se reporta la actividad a FameCheck, si el servicio está caído o falla la red, la publicación en Cwitter **no se cancela**. El usuario puede publicar normalmente y el fallo se escribe en `service_errors.log` con fecha y hora. Esto preserva la disponibilidad del sistema frente a fallos externos."

---

## 4. Diagrama Rápido de Arquitectura y Relaciones de Memoria

```text
               ┌────────────────────────────────────────────────────────┐
               │                ARQUITECTURA CWITTER                    │
               └────────────────────────────────────────────────────────┘
                                           │
         ┌─────────────────────────────────┼────────────────────────────────┐
         │                                 │                                │
         ▼                                 ▼                                ▼
┌──────────────────┐             ┌──────────────────┐             ┌──────────────────┐
│   TAD USUARIOS   │             │    TAD TWEETS    │             │   ÍNDICE ABB     │
│  (Lista Simple)  │             │  (Lista Global)  │             │  (Búsqueda ID)   │
│ Inserción: O(1)  │             │ Inserción: O(1)  │             │ Búsqueda: O(logN)│
│ Búsqueda:  O(U)  │             │ Recorrido: O(N)  │             │ Inserción:O(logN)│
└──────────────────┘             └─────────┬────────┘             └──────────────────┘
                                           │
                                           │ contiene punteros jerárquicos
                                           ▼
                                ┌────────────────────────┐
                                │      ÁRBOL N-ARIO      │
                                │ (Primer Hijo - Hermano)│
                                │  Profundidad infinita  │
                                │  DFS Pre-Order: O(K)   │
                                └────────────────────────┘
```
