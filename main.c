/**
 * =========================================================================================
 * PROYECTO: Cwitter - Red Social Académica
 * MATERIA:  Algoritmos y Estructuras de Datos (AyED 3640) - CRUI / UNLaM
 * ARCHIVO:  main.c
 * PROPÓSITO: Punto de entrada principal e interfaz de consola de usuario (CLI).
 * =========================================================================================
 * 
 * GUÍA DE COMPONENTES DEL MENÚ PARA LA DEFENSA ORAL:
 * 
 * - Opción 1 y 2: Registro y Autenticación sobre TAD ListaUsuarios (O(U)).
 * - Opción 3: Publicación de Tweet original (raíz de hilo, `id_padre = -1`, límite 280 car.).
 * - Opción 4: Respuesta jerárquica a tweet existente (Árbol N-ario, O(log N) + O(hermanos)).
 * - Opción 5: Feed Global con filtro en 2 pasadas (Prioridad Verificados en O(N)).
 * - Opción 6: Visualización completa de Hilos con indentación recursiva DFS (O(K)).
 * - Opción 7: Búsqueda lineal de tweets por texto o autor.
 * - Opción 8: Modificación de contenido de publicación propia.
 * - Opción 9: Eliminación de publicación con borrado en cascada recursivo de respuestas
 *             y reconstrucción del índice ABB (CP-30).
 * - Opción 11: Ranking Top 5 mediante Lista Simple con Inserción Ordenada (Sprint 2).
 * - Opción 12: Feed interactivo / Modo Historia con Lista Doblemente Enlazada (Sprint 2).
 * - Opción 13: Monitor de estado y logs de fallas de red FameCheck (`service_errors.log`).
 * - Opción 14: Benchmark de Búsqueda Lineal O(N) vs ABB O(log N) hasta 100k publicaciones.
 * =========================================================================================
 */

#include "include/cwitter.h"
#include <string.h>

/**
 * @brief Vacía los caracteres restantes del búfer de entrada estándar (stdin).
 * Evita saltos de lectura involuntarios causados por el carácter '\n' al alternar
 * entre `scanf` y `fgets`.
 */
static void limpiarBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * @brief Renderiza el menú principal adaptativo según el estado de la sesión.
 */
void menuPrincipal(const Usuario* logueado) {
    printf("\n======================================================================\n");
    printf("        CWITTER - RED SOCIAL (Sprint 3: Hilos & Escalabilidad)        \n");
    printf("======================================================================\n");
    if (logueado == NULL) {
        printf("1. Registrarse en Cwitter\n");
        printf("2. Iniciar Sesión\n");
    } else {
        printf("Sesión activa: @%s%s\n", logueado->nombre_usuario, logueado->es_verificado ? " [✓ Verificado]" : "");
        printf("----------------------------------------------------------------------\n");
        printf("3. Publicar Tweet Original (límite ampliado: 280 caracteres)\n");
        printf("4. Responder a un Tweet (Crear Hilo de Respuestas - Sprint 3)\n");
        printf("5. Ver Feed Global (Prioridad: Cuentas Verificadas)\n");
        printf("6. Ver Hilo Completo de un Tweet (Visualización Jerárquica DFS)\n");
        printf("7. Buscar Tweets por término o autor\n");
        printf("8. Modificar Tweet propio\n");
        printf("9. Eliminar Tweet propio (Borrado en cascada recursivo)\n");
        printf("10. Cerrar Sesión\n");
    }
    printf("----------------------------------------------------------------------\n");
    printf("11. Ranking Top 5 Usuarios Verificados (Lista Ordenada - Sprint 2)\n");
    printf("12. Modo Historia / Navegación Interactiva (Lista Doble - Sprint 2)\n");
    printf("13. Gestión de Logs y Disponibilidad de FameCheck (Sprint 2)\n");
    printf("14. Benchmark de Escalabilidad: Búsqueda Lineal vs ABB (Sprint 3)\n");
    printf("0. Salir y Guardar Datos\n");
    printf("======================================================================\n");
    printf("Seleccione una opción: ");
}

/**
 * @brief Interfaz para registrar nuevos usuarios e interactuar con FameCheck.
 */
void registrarUsuarioUI(ListaUsuarios* lista) {
    Usuario nuevo;
    printf("\n--- Registro de Nuevo Usuario ---\n");
    printf("Nombre de usuario (sin espacios): ");
    if (scanf(" %[^\n]", nuevo.nombre_usuario) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    if (strchr(nuevo.nombre_usuario, ' ') != NULL) {
        printf("Error: El nombre de usuario no puede contener espacios.\n");
        return;
    }

    if (buscarUsuario(lista, nuevo.nombre_usuario) != NULL) {
        printf("Error: El nombre de usuario '@%s' ya se encuentra registrado.\n", nuevo.nombre_usuario);
        return;
    }

    printf("Clave de acceso (sin espacios): ");
    if (scanf(" %[^\n]", nuevo.clave) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    if (strchr(nuevo.clave, ' ') != NULL) {
        printf("Error: La clave no puede contener espacios.\n");
        return;
    }

    /* Consulta al servicio externo FameCheck */
    printf("Consultando estado de verificación con servicio externo FameCheck...\n");
    nuevo.es_verificado = consultarVerificacionFameCheck(nuevo.nombre_usuario);
    if (nuevo.es_verificado) {
        printf("[FameCheck] ¡Figura pública reconocida! Cuenta clasificada como VERIFICADA [✓].\n");
    } else {
        printf("[FameCheck] Usuario estándar. Cuenta NO VERIFICADA.\n");
    }

    insertarFinalUsuario(lista, nuevo);
    printf("Usuario @%s registrado exitosamente.\n", nuevo.nombre_usuario);
}

/**
 * @brief Interfaz para publicar un tweet raíz original (hasta 280 caracteres).
 */
void publicarTweetUI(ListaTweets* lista, Usuario* usuario) {
    Tweet nuevo;
    char buffer[MAX_TWEET + 2];
    printf("\n--- Publicar Tweet Original (Límite: 280 caracteres) ---\n");
    printf("Contenido: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strlen(buffer) == 0) {
        printf("Error: El contenido del tweet no puede estar vacío.\n");
        return;
    }

    if (strlen(buffer) > MAX_TWEET) {
        printf("Error: El tweet supera el límite de 280 caracteres (longitud: %lu).\n", (unsigned long)strlen(buffer));
        return;
    }

    nuevo.id = generarSiguienteIdTweet(lista);
    nuevo.id_padre = -1; /* -1 indica tweet raíz original */
    strcpy(nuevo.autor, usuario->nombre_usuario);
    strcpy(nuevo.contenido, buffer);

    insertarFinalTweet(lista, nuevo);
    printf("¡Tweet publicado con éxito! (ID asignado: %d)\n", nuevo.id);

    /* Notificación de actividad a FameCheck si el usuario es verificado */
    if (usuario->es_verificado) {
        printf("[FameCheck] Notificando actividad de usuario verificado al servicio...\n");
        reportarPublicacionFameCheck(usuario->nombre_usuario, nuevo.id);
    }
}

/**
 * @brief Interfaz para publicar una respuesta anidada a un tweet existente.
 */
void responderTweetUI(ListaTweets* lista, Usuario* usuario) {
    int id_padre;
    char buffer[MAX_TWEET + 2];
    printf("\n--- Responder a un Tweet (Crear Hilo de Respuestas) ---\n");
    printf("Ingrese el ID del tweet al que desea responder: ");
    if (scanf("%d", &id_padre) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    /* Búsqueda rápida del tweet padre mediante índice ABB */
    Tweet* padre = buscarTweetPorId(lista, id_padre);
    if (padre == NULL) {
        printf("Error: El tweet con ID %d no existe. No se puede generar la respuesta.\n", id_padre);
        return;
    }

    printf("Tweet padre seleccionado -> [ID: %d] @%s: %s\n", padre->id, padre->autor, padre->contenido);
    printf("Ingrese su respuesta (máx 280 caracteres): ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strlen(buffer) == 0) {
        printf("Error: La respuesta no puede estar vacía.\n");
        return;
    }

    if (strlen(buffer) > MAX_TWEET) {
        printf("Error: La respuesta supera el límite de 280 caracteres (longitud: %lu).\n", (unsigned long)strlen(buffer));
        return;
    }

    Tweet respuesta;
    respuesta.id = generarSiguienteIdTweet(lista);
    respuesta.id_padre = id_padre;
    strcpy(respuesta.autor, usuario->nombre_usuario);
    strcpy(respuesta.contenido, buffer);

    if (responderTweet(lista, id_padre, respuesta)) {
        printf("¡Respuesta publicada con éxito! (ID: %d) vinculada al tweet padre ID %d.\n", respuesta.id, id_padre);

        if (usuario->es_verificado) {
            printf("[FameCheck] Notificando publicación de respuesta al servicio externo...\n");
            reportarPublicacionFameCheck(usuario->nombre_usuario, respuesta.id);
        }
    } else {
        printf("Error inesperado al enlazar la respuesta en el árbol de hilos.\n");
    }
}

/**
 * @brief Interfaz para consultar el hilo completo de un tweet raíz.
 */
void verHiloUI(const ListaTweets* lista) {
    int id_raiz;
    printf("\n--- Visualizar Hilo Completo de Conversación ---\n");
    printf("Ingrese el ID del tweet raíz: ");
    if (scanf("%d", &id_raiz) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    mostrarHiloTweet(lista, id_raiz);
}

/**
 * @brief Interfaz para modificar el texto de un tweet propio.
 */
void modificarTweetUI(ListaTweets* lista, Usuario* usuario) {
    int id;
    printf("\n--- Modificar Tweet Propio ---\n");
    printf("Ingrese el ID del tweet a editar: ");
    if (scanf("%d", &id) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    Tweet* t = buscarTweet(lista, id, usuario->nombre_usuario);
    if (t == NULL) {
        printf("Error: Tweet no encontrado o usted no es el autor del mismo.\n");
        return;
    }

    printf("Contenido actual: %s\n", t->contenido);
    printf("Nuevo contenido (máx 280 caracteres): ");

    char buffer[MAX_TWEET + 2];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (strlen(buffer) == 0) {
        printf("Error: El tweet no puede quedar vacío.\n");
        return;
    }

    if (strlen(buffer) > MAX_TWEET) {
        printf("Error: El nuevo contenido excede los 280 caracteres.\n");
        return;
    }

    strcpy(t->contenido, buffer);
    printf("Tweet ID %d modificado exitosamente.\n", id);
}

/**
 * @brief Interfaz para eliminar un tweet propio con borrado recursivo de respuestas.
 */
void eliminarTweetUI(ListaTweets* lista, Usuario* usuario) {
    int id;
    printf("\n--- Eliminar Tweet Propio (con Borrado en Cascada) ---\n");
    printf("Ingrese el ID del tweet a eliminar: ");
    if (scanf("%d", &id) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    if (eliminarTweet(lista, id, usuario->nombre_usuario)) {
        printf("Tweet ID %d y todo su árbol de respuestas fueron eliminados exitosamente.\n", id);
    } else {
        printf("Error: No se encontró el tweet especificado o no pertenece a su usuario.\n");
    }
}

/**
 * @brief Interfaz de búsqueda de tweets por coincidencia de subcadena o autor.
 */
void buscarTweetUI(const ListaTweets* lista) {
    char termino[MAX_TWEET];
    printf("\n--- Buscar Tweets ---\n");
    printf("Ingrese término de búsqueda o nombre de autor: ");
    if (scanf(" %[^\n]", termino) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    buscarTweetsPorTermino(lista, termino);
}

/**
 * @brief Interfaz para inicio de sesión.
 */
Usuario* loginUI(ListaUsuarios* lista) {
    char nombre[MAX_USUARIO];
    char clave[MAX_CLAVE];

    printf("\n--- Iniciar Sesión en Cwitter ---\n");
    printf("Usuario: ");
    if (scanf(" %[^\n]", nombre) != 1) {
        limpiarBufferEntrada();
        return NULL;
    }
    limpiarBufferEntrada();

    printf("Clave: ");
    if (scanf(" %[^\n]", clave) != 1) {
        limpiarBufferEntrada();
        return NULL;
    }
    limpiarBufferEntrada();

    Usuario* u = autenticarUsuario(lista, nombre, clave);
    if (u != NULL) {
        printf("¡Bienvenido/a de nuevo, @%s!\n", u->nombre_usuario);
        return u;
    }

    printf("Error: Credenciales incorrectas o usuario inexistente.\n");
    return NULL;
}

/**
 * @brief Interfaz para inspección de logs y simulación de fallas de red en FameCheck.
 */
void menuLogsServicioUI(void) {
    int sub_opcion;
    printf("\n--- Menú de Diagnóstico: FameCheck & Logs ---\n");
    printf("1. Ver contenido de 'service_errors.log'\n");
    printf("2. Simular caída del servicio FameCheck (Offline)\n");
    printf("3. Restaurar servicio FameCheck (Online)\n");
    printf("Estado actual del servicio: %s\n", obtenerDisponibilidadFameCheck() ? "ONLINE [Disponible]" : "OFFLINE [Caído]");
    printf("Seleccione una opción: ");

    if (scanf("%d", &sub_opcion) != 1) {
        limpiarBufferEntrada();
        return;
    }
    limpiarBufferEntrada();

    switch (sub_opcion) {
        case 1:
            mostrarLogsServicio();
            break;
        case 2:
            configurarDisponibilidadFameCheck(0);
            printf("FameCheck configurado como NO DISPONIBLE (Offline).\n");
            break;
        case 3:
            configurarDisponibilidadFameCheck(1);
            printf("FameCheck configurado como DISPONIBLE (Online).\n");
            break;
        default:
            printf("Opción inválida.\n");
    }
}

/* ========================================================================= */
/* --- FUNCIÓN PRINCIPAL MAIN ---                                           */
/* ========================================================================= */

int main(void) {
    ListaUsuarios lista_usuarios;
    ListaTweets lista_tweets;
    Usuario* usuario_logueado = NULL;
    int opcion;

    srand((unsigned int)time(NULL));

    /* 1. Inicialización de estructuras en memoria */
    inicializarListaUsuarios(&lista_usuarios);
    inicializarListaTweets(&lista_tweets);

    /* 2. Carga y reconstrucción de persistencia */
    cargarUsuarios(&lista_usuarios);
    cargarTweets(&lista_tweets);

    do {
        menuPrincipal(usuario_logueado);
        if (scanf("%d", &opcion) != 1) {
            limpiarBufferEntrada();
            break;
        }
        limpiarBufferEntrada();

        switch (opcion) {
            case 1:
                if (usuario_logueado == NULL) {
                    registrarUsuarioUI(&lista_usuarios);
                    guardarUsuarios(&lista_usuarios);
                } else {
                    printf("Ya cuenta con una sesión iniciada.\n");
                }
                break;
            case 2:
                if (usuario_logueado == NULL) {
                    usuario_logueado = loginUI(&lista_usuarios);
                    if (usuario_logueado != NULL) {
                        guardarUsuarios(&lista_usuarios);
                    }
                } else {
                    printf("Ya cuenta con una sesión iniciada.\n");
                }
                break;
            case 3:
                if (usuario_logueado != NULL) {
                    publicarTweetUI(&lista_tweets, usuario_logueado);
                    guardarTweets(&lista_tweets);
                } else {
                    printf("Debe iniciar sesión para publicar tweets.\n");
                }
                break;
            case 4:
                if (usuario_logueado != NULL) {
                    responderTweetUI(&lista_tweets, usuario_logueado);
                    guardarTweets(&lista_tweets);
                } else {
                    printf("Debe iniciar sesión para responder a un tweet.\n");
                }
                break;
            case 5:
                mostrarFeed(&lista_tweets, &lista_usuarios);
                break;
            case 6:
                verHiloUI(&lista_tweets);
                break;
            case 7:
                buscarTweetUI(&lista_tweets);
                break;
            case 8:
                if (usuario_logueado != NULL) {
                    modificarTweetUI(&lista_tweets, usuario_logueado);
                    guardarTweets(&lista_tweets);
                } else {
                    printf("Debe iniciar sesión para modificar publicaciones.\n");
                }
                break;
            case 9:
                if (usuario_logueado != NULL) {
                    eliminarTweetUI(&lista_tweets, usuario_logueado);
                    guardarTweets(&lista_tweets);
                } else {
                    printf("Debe iniciar sesión para eliminar publicaciones.\n");
                }
                break;
            case 10:
                if (usuario_logueado != NULL) {
                    printf("Sesión cerrada para el usuario @%s.\n", usuario_logueado->nombre_usuario);
                    usuario_logueado = NULL;
                } else {
                    printf("No hay ninguna sesión activa.\n");
                }
                break;
            case 11:
                mostrarTop5Verificados(&lista_usuarios, &lista_tweets);
                break;
            case 12:
                navegarFeedInteractivo(&lista_tweets, &lista_usuarios);
                break;
            case 13:
                menuLogsServicioUI();
                break;
            case 14:
                ejecutarBenchmarkBusqueda();
                break;
            case 0:
                guardarUsuarios(&lista_usuarios);
                guardarTweets(&lista_tweets);
                printf("\nDatos persistidos correctamente en disco. Saliendo de Cwitter...\n");
                break;
            default:
                printf("\nOpción no válida. Por favor, seleccione una opción del menú.\n");
        }
    } while (opcion != 0);

    /* 3. Liberación responsable de memoria dinámica (0 memory leaks) */
    liberarListaUsuarios(&lista_usuarios);
    liberarListaTweets(&lista_tweets);

    return 0;
}
