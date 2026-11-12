/**
 * =========================================================================================
 * PROYECTO: Cwitter - Red Social Académica
 * MATERIA:  Algoritmos y Estructuras de Datos (AyED 3640) - CRUI / UNLaM
 * ARCHIVO:  benchmark_busqueda.c
 * PROPÓSITO: Módulo empírico de medición y análisis de escalabilidad para el Sprint 3.
 *            Evalúa Búsqueda Lineal O(N) vs Búsqueda en Árbol Binario de Búsqueda O(log N).
 * =========================================================================================
 * 
 * GUÍA PARA LA DEFENSA ORAL (PREGUNTAS DE COLOQUIO):
 * 
 * 1. ¿Por qué se realizó este benchmark?
 *    La consigna del Sprint 3 estipula evaluar el impacto de escalar la plataforma a 100.000
 *    tweets. Antes de implementar estructuras complejas, se debe medir empíricamente si la
 *    búsqueda secuencial lineal O(N) de los Sprints 1 y 2 se degrada de manera proporcional al
 *    volumen de datos (crecimiento lineal).
 * 
 * 2. ¿Por qué se utilizó el peor caso (elemento inexistente N + 1)?
 *    Porque en el peor caso de la búsqueda lineal, el algoritmo debe recorrer TODOS y cada
 *    uno de los N nodos de la lista hasta encontrar NULL, ejecutando exactamente N comparaciones.
 *    En el ABB, el peor caso recorre una rama desde la raíz hasta una hoja nula, ejecutando
 *    a lo sumo (altura + 1) comparaciones, es decir ~ceil(log2(N + 1)).
 * 
 * 3. ¿Cuáles fueron los resultados numéricos observados?
 *    - Para N = 100:     Lineal = 100 comp.     vs ABB = 7 comp.
 *    - Para N = 1.000:   Lineal = 1.000 comp.   vs ABB = 10 comp.
 *    - Para N = 10.000:  Lineal = 10.000 comp.  vs ABB = 14 comp.
 *    - Para N = 100.000: Lineal = 100.000 comp. vs ABB = 17 comp.
 *    Reducción de esfuerzo computacional: 99.98% de operaciones ahorradas en N = 100.000.
 * =========================================================================================
 */

#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Estructura de nodo simple para simular la lista secuencial enlazada.
 */
typedef struct NodoTweetBench {
    int id;
    struct NodoTweetBench* siguiente;
} NodoTweetBench;

/**
 * @brief Estructura de nodo para el Árbol Binario de Búsqueda (ABB).
 */
typedef struct NodoABB {
    int id;
    struct NodoABB* izq;
    struct NodoABB* der;
} NodoABB;

/**
 * @brief Inserción estándar en un Árbol Binario de Búsqueda (ABB).
 * Complejidad: O(log N) promedio.
 */
NodoABB* insertarABB(NodoABB* raiz, int id) {
    if (raiz == NULL) {
        NodoABB* nuevo = (NodoABB*)malloc(sizeof(NodoABB));
        if (!nuevo) return NULL;
        nuevo->id = id;
        nuevo->izq = NULL;
        nuevo->der = NULL;
        return nuevo;
    }
    if (id < raiz->id) {
        raiz->izq = insertarABB(raiz->izq, id);
    } else if (id > raiz->id) {
        raiz->der = insertarABB(raiz->der, id);
    }
    return raiz;
}

/**
 * @brief Liberación recursiva de los nodos del ABB.
 * Complejidad: O(N).
 */
void liberarABB(NodoABB* raiz) {
    if (raiz == NULL) return;
    liberarABB(raiz->izq);
    liberarABB(raiz->der);
    free(raiz);
}

/**
 * @brief Búsqueda secuencial lineal en lista enlazada contabilizando comparaciones.
 * Complejidad: O(N).
 */
int buscarLineal(NodoTweetBench* cabeza, int id_buscado, long long* comparaciones) {
    NodoTweetBench* actual = cabeza;
    *comparaciones = 0;
    while (actual != NULL) {
        (*comparaciones)++;
        if (actual->id == id_buscado) {
            return 1;
        }
        actual = actual->siguiente;
    }
    return 0;
}

/**
 * @brief Búsqueda logarítmica en ABB contabilizando comparaciones.
 * Complejidad: O(log N).
 */
int buscarABB(NodoABB* raiz, int id_buscado, long long* comparaciones) {
    NodoABB* actual = raiz;
    *comparaciones = 0;
    while (actual != NULL) {
        (*comparaciones)++;
        if (id_buscado == actual->id) {
            return 1;
        } else if (id_buscado < actual->id) {
            actual = actual->izq;
        } else {
            actual = actual->der;
        }
    }
    return 0;
}

/**
 * @brief Construye recursivamente un ABB balanceado a partir de un rango ordenado [inicio, fin].
 * Garantiza una altura óptima h = ceil(log2(N + 1)) para evaluar el comportamiento ideal del TAD.
 * Complejidad: O(N).
 */
NodoABB* construirABBBalanceado(int inicio, int fin) {
    if (inicio > fin) return NULL;
    int medio = inicio + (fin - inicio) / 2;
    NodoABB* nodo = (NodoABB*)malloc(sizeof(NodoABB));
    if (!nodo) return NULL;
    nodo->id = medio;
    nodo->izq = construirABBBalanceado(inicio, medio - 1);
    nodo->der = construirABBBalanceado(medio + 1, fin);
    return nodo;
}

/**
 * @brief Ejecuta la prueba de rendimiento para una muestra de tamaño N.
 */
void ejecutarBenchmark(int n) {
    /* 1. Construir Lista Secuencial Enlazada con N nodos */
    NodoTweetBench* cabeza = NULL;
    NodoTweetBench* ultimo = NULL;
    for (int i = 1; i <= n; i++) {
        NodoTweetBench* nuevo = (NodoTweetBench*)malloc(sizeof(NodoTweetBench));
        if (!nuevo) break;
        nuevo->id = i;
        nuevo->siguiente = NULL;
        if (cabeza == NULL) cabeza = nuevo;
        else ultimo->siguiente = nuevo;
        ultimo = nuevo;
    }

    /* 2. Construir ABB Balanceado con N nodos */
    NodoABB* raiz_abb = construirABBBalanceado(1, n);

    /* ID para peor caso: ID inexistente al final (n + 1) */
    int id_peor_caso = n + 1;
    long long comp_lineal = 0;
    long long comp_abb = 0;

    /* 3. Medición Búsqueda Lineal (promediada con repeticiones para N pequeño) */
    int repeticiones = (n <= 1000) ? 10000 : ((n <= 10000) ? 1000 : 100);
    clock_t inicio_lineal = clock();
    for (int r = 0; r < repeticiones; r++) {
        buscarLineal(cabeza, id_peor_caso, &comp_lineal);
    }
    clock_t fin_lineal = clock();
    double tiempo_lineal_ms = ((double)(fin_lineal - inicio_lineal) * 1000.0) / (CLOCKS_PER_SEC * repeticiones);

    /* 4. Medición Búsqueda ABB */
    int repeticiones_abb = 100000;
    clock_t inicio_abb = clock();
    for (int r = 0; r < repeticiones_abb; r++) {
        buscarABB(raiz_abb, id_peor_caso, &comp_abb);
    }
    clock_t fin_abb = clock();
    double tiempo_abb_ms = ((double)(fin_abb - inicio_abb) * 1000.0) / (CLOCKS_PER_SEC * repeticiones_abb);

    printf("%-10d | %-16lld | %-14.5f | %-14lld | %-14.6f\n",
           n, comp_lineal, tiempo_lineal_ms, comp_abb, tiempo_abb_ms);

    /* 5. Liberación estricta de memoria */
    NodoTweetBench* act = cabeza;
    while (act) {
        NodoTweetBench* tmp = act;
        act = act->siguiente;
        free(tmp);
    }
    liberarABB(raiz_abb);
}

/**
 * @brief Permite compilar este archivo como binario independiente si se define BENCHMARK_STANDALONE.
 * Ejemplo de compilación: gcc -Wall -Wextra benchmark_busqueda.c -DBENCHMARK_STANDALONE -o bench.exe
 */
#ifdef BENCHMARK_STANDALONE
int main(void) {
    printf("=========================================================================================\n");
    printf("                  BENCHMARK DE ESCALABILIDAD CWITTER (SPRINT 3)                          \n");
    printf("=========================================================================================\n");
    printf("%-10s | %-16s | %-14s | %-14s | %-14s\n",
           "Tweets (N)", "Comp. Lineal", "Tiempo Lin(ms)", "Comp. ABB", "Tiempo ABB(ms)");
    printf("-----------------------------------------------------------------------------------------\n");

    ejecutarBenchmark(100);
    ejecutarBenchmark(1000);
    ejecutarBenchmark(10000);
    ejecutarBenchmark(100000);

    printf("=========================================================================================\n");
    return 0;
}
#endif
