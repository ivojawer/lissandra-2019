#include "funcionesBaseKernel.h"

extern t_list* listaTablas;
extern t_list* tiemposInsert;
extern t_list* tiemposSelect;
extern sem_t sem_tiemposInsert;
extern sem_t sem_tiemposSelect;
extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaTablas;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern t_list* memorias;
extern int proximaMemoriaEC;
extern t_log* logger;

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
	for (int i = 0; i < list_size(listaTablas); i++) {
		metadataTablaLFS* tabla = list_get(listaTablas, i);

		if (!strcmp(tabla->nombre, nombreTabla)) {
			return tabla->consistencia;
		}
	}

	return -1;
}

char* devolverTablaDeRequest(request* unaRequest) {

	char** parametros = string_split(unaRequest->parametros, " ");

	char* tabla = string_duplicate(parametros[0]);

	liberarArrayDeStrings(parametros);

	return tabla;

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

	char* nombre = string_new();

	if (script->esPorConsola) {

		string_append(&nombre, leerLinea(script->direccionScript, 0));

		char* nombreAux = nombre;

		nombre = string_substring_until(nombre, strlen(nombre) - 1);

		free(nombreAux);
	}

	else {
		string_append(&nombre,
				string_substring_from(script->direccionScript,
						sizeof(RAIZSCRIPTS) - 1));
	}

	return nombre;
}

void mostrarListaScripts(t_list* lista) {
	for (int i = 0; i < list_size(lista); i++) {
		script* script = list_get(lista, i);

		char* nombre = nombreScript(script);
		printf("%s%i%s%i%s%s%s", "Script ", script->idScript,
				": Lineas ejecutadas: ", script->lineasLeidas, ". Script: ",
				nombre, "\n");

		free(nombre);
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

void insertarTiempo(time_t tiempoInicial, time_t tiempoFinal, int request) {

	tiempoDeOperacion* tiempoOperacion = malloc(sizeof(tiempoOperacion));

	tiempoOperacion->duracion = tiempoFinal - tiempoInicial;

	tiempoOperacion->tiempoInicio = tiempoInicial;

	if (request == SELECT) {
		sem_wait(&sem_tiemposSelect);
		list_add(tiemposSelect, tiempoOperacion);
		sem_post(&sem_tiemposSelect);
	}

	else if (request == INSERT) {
		sem_wait(&sem_tiemposInsert);
		list_add(tiemposInsert, tiempoOperacion);
		sem_post(&sem_tiemposInsert);
	}
}

int esMasNuevoQue30Segundos(tiempoDeOperacion tiempoOperacion) {

	if (difftime(time(NULL), tiempoOperacion.tiempoInicio) < 30) {
		return 1;
	} else {
		return 0;
	}
}

t_list* filterCasero_esMasNuevoQue30Segundos(t_list* tiempos) {
	t_list* tiemposAux = list_create();

	tiemposAux = list_duplicate(tiempos);

	for (int i = 0; i < list_size(tiemposAux); i++) {
		tiempoDeOperacion* elemento = list_get(tiemposAux, i); //Hay que liberar esto?

		if (elemento == NULL) {
			break;
		}

		if (!esMasNuevoQue30Segundos(*elemento)) {
			list_remove(tiemposAux, i);
			i--; //Para que se mantenga en la misma posicion
		}
	}

	return tiemposAux;

}

int promedioDeTiemposDeOperaciones(t_list* tiempos) {

	if (list_size(tiempos) == 0) {
		return -1;
	}

	int sumaTotalTiempos = 0;

	for (int i = 0; i < list_size(tiempos); i++) {
		tiempoDeOperacion* tiempoOperacion = list_get(tiempos, i); //Hay que liberar esto?

		sumaTotalTiempos += tiempoOperacion->duracion;
	}

	int promedio = sumaTotalTiempos / list_size(tiempos);

	return promedio;
}

void limpiarListasTiempos() {
	t_list* listaInsertAux = tiemposInsert;
	t_list* listaSelectAux = tiemposSelect;

	sem_wait(&sem_tiemposSelect);
	tiemposSelect = filterCasero_esMasNuevoQue30Segundos(tiemposSelect);
	sem_post(&sem_tiemposSelect);

	sem_wait(&sem_tiemposInsert);
	tiemposInsert = filterCasero_esMasNuevoQue30Segundos(tiemposInsert);
	sem_post(&sem_tiemposInsert);

	list_destroy(listaInsertAux);
	list_destroy(listaSelectAux);
}

void matarListaScripts(t_list* scripts) {

	for (int i = 0; i < list_size(scripts); i++) {
		script* elemento = list_get(scripts, i);

		free(elemento->direccionScript);

		list_remove(scripts, i);
	}

	list_destroy(scripts);
}

void matarListas() {

	list_destroy(tiemposInsert);
	list_destroy(tiemposSelect);

	matarListaScripts(listaEXIT);
	matarListaScripts(listaEXEC);
	matarListaScripts(colaREADY);
	matarListaScripts(colaNEW);

}

int encontrarPosicionDeMemoria(int memoriaAEncontrar) {

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* memoria = list_get(memorias, i);

		if (memoria->nombre == memoriaAEncontrar) {
			return i;
		}
	}

	return -1;
}

int memoriaECSiguiente(int memoriaInicialEC) {

	int posicionMemoriaInicialEnLista = encontrarPosicionDeMemoria(memoriaInicialEC);

	for (int i = posicionMemoriaInicialEnLista + 1; i < list_size(memorias); i++) { // De memoriaEC a fin de lista
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->consistencias[EC] == EC) {
			return unaMemoria->nombre;
		}
	}

	for (int i = 0; i < posicionMemoriaInicialEnLista; i++) { //De comienzo de lista a memoriaEC
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->consistencias[EC] == EC) {
			return unaMemoria->nombre;
		}
	}

	return memoriaInicialEC; //Si no encuentra otra devuelve la misma
}


void enviarRequestAMemoria(request* requestAEnviar, int memoria)
{
	memoriaEnLista* memoriaALaQueEnviar = list_get(memorias,encontrarPosicionDeMemoria(memoria));
	enviarRequest(memoriaALaQueEnviar->socket, requestAEnviar);
}

int recibirRespuestaDeMemoria(int memoria)
{
	memoriaEnLista* memoriaDeQuienRecibir = list_get(memorias,encontrarPosicionDeMemoria(memoria));
	return recibirInt(memoriaDeQuienRecibir->socket,logger);
}

int determinarAQueMemoriaEnviar(int consistencia) {
	switch (consistencia) {

	case SC: {
		for (int i = 0; i < list_size(memorias); i++) {
			memoriaEnLista* unaMemoria = list_get(memorias, i);
			if (unaMemoria->consistencias[SC] == SC) {
				return unaMemoria->nombre;
			}
		}
		return -1;
	}

	case EC: {

		int memoriaAux = proximaMemoriaEC;

		proximaMemoriaEC = memoriaECSiguiente(proximaMemoriaEC);

		return memoriaAux;

	}

	case SHC:
	{
		return proximaMemoriaEC; //TODO
	}


	}

	return -1;

}

int unaMemoriaCualquiera() //TODO: Ver si tiene que tener criterio o no
{
	if (list_size(memorias) == 0)
	{
		return -1;
	}
	memoriaEnLista* unaMemoria = list_get(memorias,0);
	return unaMemoria->nombre;
}
