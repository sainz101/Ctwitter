# Plan de Pruebas - Cwitter (Sprints 1 y 2)

Este documento detalla los casos de prueba diseñados para validar las funcionalidades de Cwitter. De acuerdo con las `RULES.md`, estas pruebas se definen **antes** de la implementación.

---

## SPRINT 1: Fundamentos (Resultados de Validación)

(Se mantienen los casos del Sprint 1 para registro histórico. Al finalizar el Sprint 2, todos los "Pendiente" deben completarse).

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-01** | Plataforma vacía | Iniciar programa sin datos. | Inicia correctamente, feed vacío. | EXITOSO |
| **CP-02** | Usuario sin tweets | Ver perfil de usuario nuevo. | Informa: "No hay publicaciones". | EXITOSO |
| **CP-03** | Búsqueda inexistente | Buscar término que no existe. | Mensaje: "No hay coincidencias". | EXITOSO |
| **CP-04** | Eliminar Frente | Borrar primer tweet de la cola. | El 2do pasa a ser frente. | EXITOSO |
| **CP-05** | Eliminar Final | Borrar último tweet de la cola. | El penúltimo pasa a ser final. | EXITOSO |
| **CP-06** | Eliminar Intermedio | Borrar tweet en medio de la cola. | Punteros reasignados correctamente. | EXITOSO |
| **CP-07** | Acción sin login | Publicar/Ver feed sin sesión. | Deniega acción, pide login. | EXITOSO |
| **CP-08** | Persistencia | Reiniciar programa. | Datos cargados correctamente. | EXITOSO |

---

## SPRINT 2: Procesamiento y Verificación (Nuevos Casos)

Estos casos deben completarse al finalizar el desarrollo del Sprint 2.

### 1. Verificación y Servicio Externo (FameCheck)

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-11** | Verificación exitosa | Registrar/Loguear un usuario "famoso". | El sistema marca al usuario como "Verificado". | EXITOSO |
| **CP-12** | Verificación fallida | Registrar un usuario común. | El sistema marca al usuario como "No Verificado". | EXITOSO |
| **CP-13** | Persistencia de verificación | Reiniciar tras verificar un usuario. | El estado de verificado se mantiene sin re-consultar al servicio. | EXITOSO |
| **CP-14** | Fallo de conexión (Log) | Publicar como verificado con el servicio simulando caída. | Tweet se publica y aparece error en `service_errors.log`. | EXITOSO |

### 2. Procesamiento de Datos (Ranking y Feed)

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-15** | Top 5 con < 5 usuarios | Pedir ranking con solo 3 usuarios verificados registrados. | Muestra el ranking con 3 usuarios correctamente ordenados. | EXITOSO |
| **CP-16** | Top 5 con empate | Dos usuarios con misma cantidad de tweets. | Ambos aparecen en el ranking según orden de procesamiento. | EXITOSO |
| **CP-17** | Orden prioritario Feed | Feed con mezcla de tweets (Verificados y Normales). | Los de verificados aparecen al inicio, luego los normales. | EXITOSO |

### 3. Navegación Interactiva (Modo Historia)

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-18** | Navegación Adelante | Avanzar de a un tweet en el feed. | Se muestra el siguiente tweet correctamente. | EXITOSO |
| **CP-19** | Navegación Atrás | Retroceder tras haber avanzado. | Se muestra el tweet anterior sin recargar todo el feed. | EXITOSO |
| **CP-20** | Límites del Feed | Intentar ir atrás en el 1er tweet o adelante en el último. | El sistema avisa que no hay más elementos en esa dirección. | EXITOSO |

---

## Casos de Prueba Propios (Sprint 2)

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-21** | Reinicio durante navegación | Iniciar modo historia y cerrar el programa. | Al volver, el sistema debe permitir iniciar la navegación desde el principio sin errores. | EXITOSO |
| **CP-22** | Tweet vacío en navegación | Navegar por un feed donde hay tweets con contenido mínimo. | El sistema visualiza correctamente incluso tweets muy cortos o solo espacios. | EXITOSO |

---

## Notas de Validación
*   **Fugas de Memoria:** Crítico verificar la liberación de la Lista Doblemente Enlazada generada para el feed.
*   **Logs:** Verificar que el archivo `service_errors.log` no se sobrescriba, sino que acumule (append).


---

## SPRINT 3: Hilos, Escalabilidad y Rendimiento (Nuevos Casos)

Estos casos de prueba se definen previo a la implementación del Sprint 3 para evaluar los límites del sistema y la robustez de las nuevas estructuras arbóreas y de búsqueda.

| ID | Situación a probar | Operación realizada | Resultado esperado | Salida obtenida |
|:---|:---|:---|:---|:---|
| **CP-23** | Límite ampliado de 280 caracteres | Intentar publicar tweet de 280 caracteres (válido) y luego uno de 281 (excedido). | El tweet de 280 se publica con éxito; el de 281 es rechazado con mensaje de error. | EXITOSO |
| **CP-24** | Respuesta directa (Hilo Nivel 1) | Responder a un tweet raíz existente indicando su ID de publicación. | Se genera el nuevo tweet asociado a `id_padre`, manteniéndose en la lista global e indexado. | EXITOSO |
| **CP-25** | Respuestas multinivel (Profundidad N) | Publicar respuesta a una respuesta previa (Nivel 2) y una tercera a esta última (Nivel 3). | Se encadenan los punteros jerárquicos formando una rama de profundidad 3 sin desbordes. | EXITOSO |
| **CP-26** | Respuesta a ID inexistente | Intentar responder indicando un ID de tweet padre que no existe en el sistema. | Se rechaza la publicación, informando que el tweet padre no fue encontrado. El sistema se mantiene estable. | EXITOSO |
| **CP-27** | Visualización jerárquica del hilo | Seleccionar un tweet raíz con múltiples respuestas directas y anidadas para ver su hilo. | Se despliega el árbol completo con sangrías y conectores visuales según el nivel de profundidad. | EXITOSO |
| **CP-28** | Persistencia y reconstrucción | Publicar un hilo multinivel, salir de Cwitter y reiniciar el programa. | Los hilos se recargan desde `tweets.txt` reconstruyendo idénticamente la jerarquía de respuestas. | EXITOSO |
| **CP-29** | Búsqueda escalable masiva | Buscar tweets por ID sobre una colección indexada con Árbol Binario de Búsqueda (ABB). | Búsqueda exitosa en tiempo logarítmico $O(\log N)$, resolviendo en menos de 20 comparaciones. | EXITOSO |
| **CP-30** | Eliminación y memoria recursiva | Eliminar un tweet que contiene respuestas y verificar liberación de memoria. | Se elimina la publicación y se liberan recursivamente los nodos de respuestas sin memory leaks. | EXITOSO |
