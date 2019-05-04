#include "funcionesBaseKernel.h"

extern t_list* listaTablas;

int encontrarScriptEnLista(int id, t_list* lista) {
	for (int i = 0; i < list_size(lista); i++) {

		script* script = list_get(lista, i);
		if (script->idScript == id) {
			return i;
		}

	}
	return -1;
}

int removerScriptDeLista(int id, t_list* lista) {

	int index = encontrarScriptEnLista(id, lista);

	if (index == -1) {
		return -1;
	}

	list_remove(lista, index);

	return 1;
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

char* devolverTablaDeRequest(request* request) {

	char** parametros = string_split(request->parametros, " ");

	char* tabla = string_duplicate(parametros[0]);

	liberarArrayDeStrings(parametros);

	return tabla;

}

int esDescribeGlobal(request* request) {
	int resultado = 0;

	if (request->requestEnInt == DESCRIBE
			&& (!strcmp(" ", request->parametros))) {
		resultado = 1;
	}

	return resultado;
}

char* scriptConRaiz(char* script) {
	char * dirScript = string_new();
	string_append(&dirScript, RAIZSCRIPTS);
	string_append(&dirScript, script);
	char * resultado = string_duplicate(dirScript);
	free(dirScript);
	return resultado;
}

int existeArchivo(char* script) {
	FILE* archivo;

	char* direccion = scriptConRaiz(script);

	archivo = fopen(direccion, "r");

	if (archivo == NULL) {
		fclose(archivo);
		return 0;
	}

	fclose(archivo);
	free(direccion);

	return 1;
}

void limpiarBuffer(char* buffer) {
	for (int i = 0; i < MAXBUFFER; i++) {
		buffer[i] = '\n';
	}
}

int charsDeBuffer(char* buffer) {
	int caracteres = 0;
	for (; caracteres < MAXBUFFER; caracteres++) {
		if (buffer[caracteres] == '\n') {
			break;
		}
	}

	return caracteres + 1;
}

char* leerLinea(char* direccion, int lineaALeer) {

	//TODO: Arreglar esto

	//TODO: Semaforo con crearArchivo?

	FILE* archivo;
	archivo = fopen(direccion, "r");
	char buffer[MAXBUFFER];

	if (archivo == NULL) {
		fclose(archivo);

		char* error = "error";

		return error;
	}

	for (int i = 0; i <= lineaALeer; i++) {
		limpiarBuffer(buffer);
		char* resultado = fgets(buffer, MAXBUFFER, archivo);

		if (resultado == NULL) {
			fclose(archivo);
			free(resultado);

			char* fin = "fin";

			return fin;
		}

	}

	int caracteres = charsDeBuffer(buffer);

	char* linea = malloc(sizeof(char) * caracteres);

	memcpy(linea, buffer, caracteres * sizeof(char));

	fclose(archivo);

	return linea;
}



int crearArchivoParaRequest(script* script, request* requestAArchivo) {

	FILE* archivo;
	char* direccion = scriptConRaiz(string_itoa(script->idScript));

	archivo = fopen(direccion, "w");

	if (archivo == NULL) {
		fclose(archivo);
		return -1;
	}

	if (fputs(requestStructAString(requestAArchivo), archivo) == EOF) {
		fclose(archivo);
		return -1;
	}

	script->direccionScript = malloc(sizeof(direccion));
	script->direccionScript = direccion;

	fclose(archivo);

	return 1;

}

char* nombreScript(script* script) {
	if (script->esPorConsola) {
		return leerLinea(script->direccionScript, 0);
	}

	return string_substring_from(script->direccionScript,
			sizeof(RAIZSCRIPTS) - 1);
}

void mostrarListaScripts(t_list* lista) {
	for (int i = 0; i < list_size(lista); i++) {
		script* script = list_get(lista, i);
		printf("%s%i%s%i%s%s%s", "Script ", script->idScript,
				": Lineas ejecutadas: ", script->lineasLeidas, ". Script: ",
				nombreScript(script), "\n");
	}
}

int moverScript(int scriptID, t_list* listaOrigen, t_list* listaDestino) {

	//TODO: Poner semaforos?

	int index = encontrarScriptEnLista(scriptID, listaOrigen);

	if (index == -1) {
		return -1;
	}

	script* script = list_get(listaOrigen, index);

	if ((removerScriptDeLista(scriptID, listaOrigen)) == 0) {
		return 0;
	}

	list_add(listaDestino, script);
	return 1;
}

