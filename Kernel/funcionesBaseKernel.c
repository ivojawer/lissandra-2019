#include "funcionesBaseKernel.h"

extern t_log* logger;
extern t_list* listaTablas;
extern t_list* tiemposInsert;
extern t_list* tiemposSelect;
extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaTablas;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern sem_t sem_tiemposInsert;
extern sem_t sem_tiemposSelect;
extern sem_t sem_actualizacionMetadatas;

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

char* leerLinea(char* direccion, int lineaALeer) {

	FILE* archivo;
	archivo = fopen(direccion, "r");
	char* resultado = string_new();

	if (archivo == NULL) {

		string_append(&resultado, "fin");

		return resultado;
	}

	for (int i = 0; i <= lineaALeer; i++) {

		char buffer[MAXBUFFER];

		char* resultadoDeLeer = fgets(buffer, MAXBUFFER, archivo);

		if (resultadoDeLeer == NULL) {

			string_append(&resultado, "fin");

			free(resultadoDeLeer);

			break;

		}

		if (i == lineaALeer) {

			string_append(&resultado, resultadoDeLeer);

			char* resultadoInverso = string_reverse(resultado);

			if (resultadoInverso[0] == '\n') { //Si tiene un \n al final

				char* resultadoAux = resultado;
				resultado = string_substring_until(resultado,
						strlen(resultado) - 1); //Se saca el \n
				free(resultadoAux);
			}

			free(resultadoInverso);

		}

	}

	fclose(archivo);

	return resultado;

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

	t_list* tiemposAux = list_duplicate(tiempos);

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
	sem_wait(&sem_tiemposSelect);
	sem_wait(&sem_tiemposInsert);
	t_list* listaInsertAux = tiemposInsert;
	t_list* listaSelectAux = tiemposSelect;

	tiemposSelect = filterCasero_esMasNuevoQue30Segundos(tiemposSelect);

	tiemposInsert = filterCasero_esMasNuevoQue30Segundos(tiemposInsert);

	list_destroy(listaInsertAux);
	list_destroy(listaSelectAux);
	sem_post(&sem_tiemposSelect);
	sem_post(&sem_tiemposInsert);
}


void matarListas() {

	list_destroy(tiemposInsert);
	list_destroy(tiemposSelect);

	matarListaScripts(listaEXIT);
	matarListaScripts(listaEXEC);
	matarListaScripts(colaREADY);
	matarListaScripts(colaNEW);

}



void agregarUnaMetadata(metadataTablaLFS* unaMetadata) {
	if (criterioDeTabla(unaMetadata->nombre) == -1) //Si no existe ya
			{
		sem_wait(&sem_actualizacionMetadatas);
		list_add(listaTablas, unaMetadata);
		sem_post(&sem_actualizacionMetadatas);
		log_info(logger, "Se agrego la tabla %s a las metadatas",
				unaMetadata->nombre);
	} else {
		free(unaMetadata->nombre);
		free(unaMetadata);
	}
}

void actualizarMetadatas(t_list* metadatas) {

	sem_wait(&sem_actualizacionMetadatas);

	while (list_size(listaTablas) != 0) {
		metadataTablaLFS* unaMetadata = list_remove(listaTablas, 0);

		free(unaMetadata->nombre);
		free(unaMetadata);
	}

	list_destroy(listaTablas);

	listaTablas = metadatas;

	log_info(logger, "Se actualizaron las metadatas.");

	sem_post(&sem_actualizacionMetadatas);
}

void agregarUnaMetadataEnString(char* metadataEnString) {
	char** parametrosMetadata = string_split(metadataEnString, " ");

	char* nombreTabla = string_duplicate(parametrosMetadata[0]);
	int laConsistencia = queConsistenciaEs(parametrosMetadata[1]);
	int lasParticiones = atoi(parametrosMetadata[2]);
	int elCompactTime = atoi(parametrosMetadata[3]);

	metadataTablaLFS* unaMetadata = malloc(sizeof(metadataTablaLFS));

	unaMetadata->nombre = nombreTabla;
	unaMetadata->consistencia = laConsistencia;
	unaMetadata->compactTime = elCompactTime;
	unaMetadata->particiones = lasParticiones;

	agregarUnaMetadata(unaMetadata);

	liberarArrayDeStrings(parametrosMetadata);

}

