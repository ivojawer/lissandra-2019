#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaTablas;
extern int idInicial;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;

int removerScriptDeLista(int id, t_list* lista) {
	for (int i; i < list_size(lista); i++) {
		script* script = list_get(lista, i);

		if (script->idScript == id) {
			if (script->direccionScript != NULL) {
				free(script->direccionScript);
			}

			free(script);

			list_remove(lista, i);

			return 1;
		}
	}
	return 0;
}

int encontrarScriptEnLista(int id, t_list* lista) {
	for (int i; i < list_size(lista); i++) {

		script* script = list_get(lista, i);
		if (script->idScript == id) {
			return i;
		}

	}
	return -1;
}

int criterioDeTabla(char* nombreTabla) {
	for (int i; i < list_size(listaTablas); i++) {
		tablaEnLista* tabla = list_get(listaTablas, i);

		if (!strcmp(tabla->nombreTabla, nombreTabla)) {
			return tabla->consistencia;
		}
	}

	return -1;
}

char* devolverTablaDeRequest(char* request) {

	char** requestYParametros = string_split(request, " ");

	char* tabla = string_duplicate(requestYParametros[1]);

	liberarArrayDeStrings(requestYParametros);

	return tabla;

}

int esDescribeGlobal(char* request) {
	int resultado = 0;

	char** requestYParametros = string_split(request, " ");

	int requestEnInt = queRequestEs(requestYParametros[0]);

	if (requestEnInt == DESCRIBE && (requestYParametros[1] == NULL)) {
		resultado = 1;
	}

	liberarArrayDeStrings(requestYParametros);

	return resultado;
}

int existeArchivo(char* direccion) //TODO
{
	return 0;
}

void crearScript(parametros_hiloScript* parametros) {
	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));

	sem_wait(&sem_cambioId);
	nuevoScript->idScript = idInicial;
	idInicial++;
	sem_post(&sem_cambioId);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	nuevoScript->lineasLeidas = 0;

	if (parametros->request == RUN) {

		nuevoScript->direccionScript = malloc(sizeof(parametros->parametros));
		nuevoScript->direccionScript = string_duplicate(parametros->parametros);
	}

	else {	//TODO:
			//Crear archivo script de una linea
			//Poner direccion en el script
	}

	removerScriptDeLista(nuevoScript->idScript, colaNEW);
	list_add(colaREADY, nuevoScript);

	sem_post(&sem_disponibleColaREADY);

}

char* leerLinea(char* direccion,int lineaALeer) //TODO
{

	return "Hola";
}

int ejecutarRequest(char* request) {

	int requestEnInt = devolverSoloRequest(request);

	switch (requestEnInt) {

	case JOURNAL: {
		;
		//Hacer el journal
		return -1;
	}

	case RUN: {
		;
		char** requestYParametros = string_split(request, " ");

		if (!existeArchivo(requestYParametros[1])) {
			liberarArrayDeStrings(requestYParametros);
			return -1;
		}
		parametros_hiloScript* parametros = malloc(
				sizeof(parametros_hiloScript));

		parametros->request = RUN;
		parametros->parametros = string_duplicate(requestYParametros[1]);

		crearScript(parametros);

		liberarArrayDeStrings(requestYParametros);
		return 1;
	}

	case METRICS: {
		;
		//Hacer el metrics
		return -1;
	}

	case ADD: {
		;
		//Hacer el add
		return -1;
	}

	}

	if (esDescribeGlobal(request)) {
		//Hacer el describe global
		return -1;
	}

	int criterio = criterioDeTabla(devolverTablaDeRequest(request));

	//por aca enviar :)

	return -1;
}

