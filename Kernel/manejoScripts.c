#include "funcionesBaseKernel.h"

extern sem_t sem_movimientoScripts;


script* encontrarScriptEnLista(int id, t_list* lista) { //Si no se encuentra, se devuelve idScript = -1
	sem_wait(&sem_movimientoScripts);
	for (int i = 0; i < list_size(lista); i++) {

		script* script = list_get(lista, i);
		if (script->idScript == id) {

			sem_post(&sem_movimientoScripts);
			return script;
		}

	}
	script* scriptFallido = malloc(sizeof(script));

	scriptFallido->idScript = -1;

	sem_post(&sem_movimientoScripts);
	return scriptFallido;
}

int removerScriptDeLista(int id, t_list* lista) {

	for (int i = 0; i < list_size(lista); i++) {

		script* script = list_get(lista, i);
		if (script->idScript == id) {
			list_remove(lista, i);
			return 1;
		}

	}
	return -1;
}

char* scriptConRaiz(char* script) { //No se por que se hace el duplicate y el free aca ¿¿??
	char * dirScript = string_new();
	string_append(&dirScript, RAIZSCRIPTS);
	string_append(&dirScript, script);
	char * resultado = string_duplicate(dirScript);
	free(dirScript);
	return resultado;
}

int crearArchivoParaRequest(script* script, request* requestAArchivo) {

	FILE* archivo;
	char* direccion = scriptConRaiz(string_itoa(script->idScript));

	archivo = fopen(direccion, "w");

	if (archivo == NULL) {
//		fclose(archivo);
		return -1;
	}

	if (fputs(requestStructAString(requestAArchivo), archivo) == EOF) {
		fclose(archivo);
		return -1;
	}

//	script->direccionScript = malloc(sizeof(direccion)); Esto se saco para se reemplazado por las 2 lineas de abajo
//	script->direccionScript = direccion;

	script->direccionScript = string_duplicate(direccion);
	free(direccion);

	fclose(archivo);

	return 1;

}

char* nombreScript(script* script) {

	char* nombre = string_new();

	if (script->esPorConsola) {

		if (strlen(script->direccionScript) > strlen(RAIZSCRIPTS)) {
			char* nombreAux = string_substring_from(script->direccionScript,
					sizeof(RAIZSCRIPTS) - 1);

			if (!strcmp(nombreAux, string_itoa(script->idScript))) {
				string_append(&nombre, leerLinea(script->direccionScript, 0));
			} else {
				string_append(&nombre, script->direccionScript);
			}
		} else {
			string_append(&nombre, script->direccionScript);
		}

	}

	else {
		string_append(&nombre,
				string_substring_from(script->direccionScript,
						sizeof(RAIZSCRIPTS) - 1));
	}

	return nombre;
}

void mostrarListaScripts(t_list* lista) {

	sem_wait(&sem_movimientoScripts);

	for (int i = 0; i < list_size(lista); i++) {
		script* script = list_get(lista, i);

		char* nombre = nombreScript(script);
		printf("ID script %i: Lineas ejecutadas: %i. Script: %s\n",
				script->idScript, script->lineasLeidas, nombre);

		free(nombre);
	}

	sem_post(&sem_movimientoScripts);
}

int moverScript(int scriptID, t_list* listaOrigen, t_list* listaDestino) {

	script* unScript = encontrarScriptEnLista(scriptID, listaOrigen);

	if (unScript->idScript == -1) { //Es un script fallido, se puede hacerle free
		free(unScript);
		return -1;
	}

	sem_wait(&sem_movimientoScripts);
	script* script = unScript;

	if ((removerScriptDeLista(scriptID, listaOrigen)) == -1) {
		sem_post(&sem_movimientoScripts);
		return -1;
	}

	list_add(listaDestino, script);
	sem_post(&sem_movimientoScripts);

	return 1;
}

void matarListaScripts(t_list* scripts) {

	while (list_size(scripts) != 0) {
		script* elemento = list_remove(scripts, 0);

		free(elemento->direccionScript);
	}

	list_destroy(scripts);
}

void sacarScriptDeEspera(int nombreScript, memoriaEnLista* laMemoria) {
	for (int i = 0; i < list_size(laMemoria->scriptsEsperando); i++) {
		int* unScript = list_get(laMemoria->scriptsEsperando, i);

		if (nombreScript == *unScript) {
			list_remove(laMemoria->scriptsEsperando, i);
			return;
		}
	}
}
