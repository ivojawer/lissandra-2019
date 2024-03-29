#include "funcionesBaseKernel.h"
extern t_log* logger;
extern t_list* listaTablas;
extern sem_t sem_actualizacionMetadatas;

int metadataExiste(char*nombreMetadata) {
	if (criterioDeTabla(nombreMetadata) == -1) {
		return 0;
	} else {
		return 1;
	}
}

void removerMetadataDeUnRequest(request* unaRequest) {
	char** parametrosDeLaRequest = string_split(unaRequest->parametros, " ");
	char* tabla = parametrosDeLaRequest[0];

	removerUnaMetadata(tabla);

	liberarArrayDeStrings(parametrosDeLaRequest);
}

void sobreescribirMetadata(metadataTablaLFS* unaMetadata) {
	sem_wait(&sem_actualizacionMetadatas);

	for (int i = 0; i < list_size(listaTablas); i++) {
		metadataTablaLFS* tabla = list_get(listaTablas, i);

		if (!strcmp(tabla->nombre, unaMetadata->nombre)) {
			free(tabla->nombre);
			free(tabla);
			tabla = unaMetadata;
			sem_post(&sem_actualizacionMetadatas);
			return;
		}
	}

	list_add(listaTablas, unaMetadata);

	sem_post(&sem_actualizacionMetadatas);
}

void agregarUnaMetadata(metadataTablaLFS* unaMetadata) {

	int metadataExisteSinSincro() {
		for (int i = 0; i < list_size(listaTablas); i++) {
			metadataTablaLFS* tabla = list_get(listaTablas, i);

			if (!strcmp(tabla->nombre, unaMetadata->nombre)) {
				return 1;
			}
		}
		return 0;
	}

	sem_wait(&sem_actualizacionMetadatas);

	if (!metadataExisteSinSincro()) {

		list_add(listaTablas, unaMetadata);

		char* textoALoggear = string_new();
		string_append(&textoALoggear, "Se agrego la tabla ");
		string_append(&textoALoggear, unaMetadata->nombre);
		string_append(&textoALoggear, " a las metadatas");
		loggearAmarillo(logger, textoALoggear);
		free(textoALoggear);

	} else {
		free(unaMetadata->nombre);
		free(unaMetadata);
	}

	sem_post(&sem_actualizacionMetadatas);
}

void removerUnaMetadata(char* nombreMetadata) {
	sem_wait(&sem_actualizacionMetadatas);

	for (int i = 0; i < list_size(listaTablas); i++) {
		metadataTablaLFS* unaMetadata = list_get(listaTablas, i);

		if (!strcmp(unaMetadata->nombre, nombreMetadata)) {

			char* textoALoggear = string_new();
			string_append(&textoALoggear, "Se removio la tabla ");
			string_append(&textoALoggear, unaMetadata->nombre);
			string_append(&textoALoggear, " de la lista interna");
			loggearAmarillo(logger, textoALoggear);
			free(textoALoggear);

			list_remove(listaTablas, i);
			free(unaMetadata->nombre);
			free(unaMetadata);
			sem_post(&sem_actualizacionMetadatas);

			return;
		}
	}

	sem_post(&sem_actualizacionMetadatas);
}

void actualizarMetadatas(t_list* metadatas) {

	sem_wait(&sem_actualizacionMetadatas);

	while (list_size(listaTablas) != 0) { //Se remueven todos los elementos de la lista
		metadataTablaLFS* unaMetadata = list_remove(listaTablas, 0);

		free(unaMetadata->nombre);
		free(unaMetadata);
	}

	list_destroy(listaTablas);

	listaTablas = metadatas; //Se reemplaza la lista original por la nueva lista

	loggearAmarillo(logger, "Se actualizaron las metadatas.");

	sem_post(&sem_actualizacionMetadatas);
}

void agregarUnaMetadataEnString(char* metadataEnString) {

	char* textoALoggear = string_new();
	string_append(&textoALoggear, "Se agrego la metadata ");
	string_append(&textoALoggear, metadataEnString);
	loggearAmarillo(logger, textoALoggear);
	free(textoALoggear);

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
