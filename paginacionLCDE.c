//López Cantarell Diego Emir N.Cta:318283195
//Sistemas Operativos 
//Objetivo
//Elaborar un programa en ANSI C que efectúe la administración de memoria por paginación,
//utilizando un vector “de áreas libres” (el que usa Linux) tanto para la asignación como para
//liberación de marcos de página de memoria real.

//librerias a utilizar:
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>

//definimos el tamaño del mapa de memoria 
//y del vector de areas libres
#define TOTAL_FRAMES 16  
#define FREE_AREAS_SIZE 5

//definimos la estructura del nodo, cada nodo en la lista
//que almacena las áreas libres de memoria
typedef struct Nodo {
    int inicio;
    int tamaño;
    struct Nodo* siguiente;
} Nodo;

//Funcion que imprime el mapa de memoria cada que se actualize
//para mostras los marcos de página asignados a cada proceso
//y los que se encuentran libres
void imprimirMapaDeMemoria(int mapaDeMemoria[]) {
    printf("\nMapa de Memoria:\n");
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (mapaDeMemoria[i] == -1)
            printf("[ ]");
        else
            printf("[%d]", mapaDeMemoria[i]);
    }
    printf("\n");
}


//Funcion que imprime las areas libres de memoria,
//mostrando la lista enlazada de areas libres y sus tamaños
void imprimirAreasLibres(Nodo* areasLibres[]) {
    printf("\nÁreas Libres:\n");
    for (int i = 0; i < FREE_AREAS_SIZE; i++) {  
        printf("[%d] -> ", i);
        Nodo* actual = areasLibres[i];
        while (actual != NULL) {
            printf("(%d, %d) -> ", actual->inicio, actual->tamaño);
            actual = actual->siguiente;
        }
        printf("NULL\n");
    }
    printf("\n");
}


//funcion que crea nodos de área libre y le asigna
//valores a los campos de inicio y tamaño
Nodo* crearNodoAreaLibre(int inicio, int tamaño) {
    Nodo* nuevoNodo = (Nodo*)malloc(sizeof(Nodo));
    nuevoNodo->inicio = inicio;
    nuevoNodo->tamaño = tamaño;
    nuevoNodo->siguiente = NULL;
    return nuevoNodo;
}


//esta funcion inserta nuevos nodos de área libre
//a la lista enlazada de áreas libres 
void insertarAreaLibre(Nodo** cabeza, Nodo* nuevoNodo) {
    if (*cabeza == NULL) {
        *cabeza = nuevoNodo;
        return;
    }
    if (nuevoNodo->inicio < (*cabeza)->inicio) {
        nuevoNodo->siguiente = *cabeza;
        *cabeza = nuevoNodo;
        return;
    }
    Nodo* actual = *cabeza;
    while (actual->siguiente != NULL && actual->siguiente->inicio < nuevoNodo->inicio) {
        actual = actual->siguiente;
    }
    nuevoNodo->siguiente = actual->siguiente;
    actual->siguiente = nuevoNodo;
}


//si es necesario eliminar nodos de área libre
//esta funcion lo realiza y se refleja en la lista enlazada
//de áreas libres
void eliminarAreaLibre(Nodo** cabeza, Nodo* objetivo) {
    if (*cabeza == NULL)
        return;
    if (*cabeza == objetivo) {
        *cabeza = (*cabeza)->siguiente;
        free(objetivo);
        return;
    }
    Nodo* actual = *cabeza;
    while (actual != NULL && actual->siguiente != objetivo) {
        actual = actual->siguiente;
    }
    if (actual != NULL) {
        actual->siguiente = objetivo->siguiente;
        free(objetivo);
    }
}


//de ser necesario fusionar areas libres contiguas en nuestra
//lista enlaza. Comprueba si dos nodos adyacentes en la lista
//pueden fusionarse en un solo nodo más grande
void fusionarAreasLibres(Nodo* areasLibres[]) {
    for (int i = 0; i < FREE_AREAS_SIZE - 1; i++) {
        Nodo* actual = areasLibres[i];
        while (actual != NULL && actual->siguiente != NULL) {
            if (actual->inicio + actual->tamaño == actual->siguiente->inicio) {
                actual->tamaño += actual->siguiente->tamaño;
                eliminarAreaLibre(&areasLibres[i + 1], actual->siguiente);
            } else {
                actual = actual->siguiente;
            }
        }
    }
}


//Esta funcion toma el identificador del proceso y el número de 
//paginas a asignar para asignar la memoria necesaria
void asignarMemoria(int mapaDeMemoria[], Nodo* areasLibres[], int idProceso, int numPaginas) {
    printf("Asignando %d páginas para el proceso %d\n", numPaginas, idProceso);

    int tamañoRequerido = (int)ceil(log2(numPaginas));
    int contadorAsignado = 0;
    int i = 0;
    while (contadorAsignado < numPaginas && i < FREE_AREAS_SIZE) {
        Nodo* actual = areasLibres[i];
        while (actual != NULL && contadorAsignado < numPaginas) {
            int inicio = actual->inicio;
            int tamaño = actual->tamaño;
            int tamañoActual = (int)pow(2, (int)ceil(log2(tamaño)));
            if (tamañoActual >= tamañoRequerido) {
                for (int j = inicio; j < inicio + numPaginas; j++) {
                    mapaDeMemoria[j] = idProceso;
                }
                contadorAsignado = numPaginas;
                if (tamañoActual > numPaginas) {
                    actual->inicio += numPaginas;
                    actual->tamaño -= numPaginas;
                } else {
                    eliminarAreaLibre(&areasLibres[i], actual);
                }
            } else {
                actual = actual->siguiente;
            }
        }
        i++;
    }

    if (contadorAsignado == numPaginas) {
        printf("Asignación de memoria exitosa.\n");
        imprimirMapaDeMemoria(mapaDeMemoria);
        imprimirAreasLibres(areasLibres);
    } else {
        printf("Memoria libre insuficiente.\n");
    }
}


//cuando reciba un marco -1 esta funcion toma el identificador
//del proceso y libera los marcos de página asignados 
//a este proceso
void desasignarMemoria(int mapaDeMemoria[], Nodo* areasLibres[], int idProceso) {
    printf("Desasignando memoria para el proceso %d\n", idProceso);

    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (mapaDeMemoria[i] == idProceso) {
            mapaDeMemoria[i] = -1;
        }
    }


    int inicio = 0;
    int tamaño = 0;
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (mapaDeMemoria[i] == -1) {
            if (tamaño == 0) {
                inicio = i;
            }
            tamaño++;
        } else if (tamaño > 0) {
            insertarAreaLibre(&areasLibres[FREE_AREAS_SIZE - 1], crearNodoAreaLibre(inicio, tamaño));
            tamaño = 0;
        }
    }
    if (tamaño > 0) {
        insertarAreaLibre(&areasLibres[FREE_AREAS_SIZE - 1], crearNodoAreaLibre(inicio, tamaño));
    }

    fusionarAreasLibres(areasLibres);

    printf("Desasignación de memoria exitosa.\n");
    imprimirMapaDeMemoria(mapaDeMemoria);
    imprimirAreasLibres(areasLibres);
}


//en la funcion principal leeremos el archivo de entrada
//con la información sobre asignación de memoria y realiza las
//operaciones correspondientes llamando a las funciones necesarias
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo_entrada>\n", argv[0]);
        return 1;
    }

    char* archivoEntrada = argv[1];
    FILE* archivo = fopen(archivoEntrada, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo de entrada.\n");
        return 1;
    }

    int mapaDeMemoria[TOTAL_FRAMES];
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        mapaDeMemoria[i] = -1;
    }

    Nodo* areasLibres[FREE_AREAS_SIZE];
    for (int i = 0; i < FREE_AREAS_SIZE; i++) {
        areasLibres[i] = NULL;
    }
    areasLibres[4] = crearNodoAreaLibre(0, 16);

    imprimirMapaDeMemoria(mapaDeMemoria);
    imprimirAreasLibres(areasLibres);

    int idProceso, accion;
    while (fscanf(archivo, "%d %d", &idProceso, &accion) != EOF) {
        if (accion > 0) {
            asignarMemoria(mapaDeMemoria, areasLibres, idProceso, accion);
        } else if (accion == -1) {
            desasignarMemoria(mapaDeMemoria, areasLibres, idProceso);
        }
    }

    fclose(archivo);
    return 0;
}
//Conclusion:
//López Cantarell Diego Emir
//Al elaborar el programa que simula la administración
//de memoria por paginación pude entender de mejor manera como trabaja
//el SO para administrar la memoria en LINUX, algo que hasta estos dias no entendia
//y para mi solo era un simple manejo de memoria
//entender la complejidad de como trabaja y hace las asignaciones me sirvio para 
//entender como el SO trabaja para aprovechar la memoria de la mejor manera posible.