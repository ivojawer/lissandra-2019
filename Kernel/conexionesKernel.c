#include "funcionesKernel.h"

extern t_log* logger;
extern t_list* memorias;
extern sem_t sem_gossiping;
extern t_list* listaEXEC;

void conectarseAUnaMemoria(seed* unaSeed) {

	int socketMemoria = conectarseAServidor(unaSeed->ip, unaSeed->puerto);

	if (socketMemoria == -1) {
		return;
	}

	enviarIntConHeader(KERNEL, HANDSHAKE, socketMemoria);

	int headerRespuesta = recibirInt(socketMemoria, logger);

	if (headerRespuesta != HANDSHAKE) {
		log_error(logger, "Se envio un handshake y se devolvio otra cosa.");
		return;
	}

	int modulo = recibirInt(socketMemoria, logger);

	if (modulo != MEMORIA) {
		log_error(logger, "Se conecto algo que no es la memoria.");
		return;
	}

	int nombreMemoria = recibirInt(socketMemoria, logger);

	if (nombreMemoria == -1) {
		return; //El error ya esta en el recibirInt
	}

	memoriaEnLista* nuevaMemoria = malloc(sizeof(memoriaEnLista));

	int* nuevaConsistencia = malloc(sizeof(int) * 3);
	nuevaConsistencia[SC] = -1;
	nuevaConsistencia[SHC] = -1;
	nuevaConsistencia[EC] = -1;

	nuevaMemoria->nombre = nombreMemoria;
	nuevaMemoria->socket = socketMemoria;
	nuevaMemoria->consistencias = nuevaConsistencia;

	nuevaMemoria->ip = string_duplicate(unaSeed->ip);
	nuevaMemoria->puerto = unaSeed->puerto;

	free(unaSeed->ip);

	free(unaSeed);

	list_add(memorias, nuevaMemoria);

	log_info(logger, "Se acaba de conectar la memoria %i",
			nuevaMemoria->nombre);

	comunicacionConMemoria(nuevaMemoria);

}

void manejoErrorMemoria(int nombreMemoria) {
	log_error(logger,
			"Se recibio de la memoria %i algo incorrecto, se va a cerrar la conexion.",
			nombreMemoria);
	matarMemoria(nombreMemoria);
}

void agregarMemorias(t_list* seeds) {

	sem_wait(&sem_gossiping);

	while (list_size(seeds) != 0) {

		seed* unaSeed = list_remove(seeds, 0);

		if (seedYaExiste(unaSeed)) {
			free(unaSeed->ip);
			free(unaSeed);
			continue;
		}

		pthread_t h_nuevaConexion;

		pthread_create(&h_nuevaConexion, NULL, (void *) conectarseAUnaMemoria,
				seeds);

		pthread_detach(h_nuevaConexion);

	}

	list_destroy(seeds);

	sem_post(&sem_gossiping);

}

void comunicacionConMemoria(memoriaEnLista* memoria) {
	int socketMemoria = memoria->socket;

	while (1) {
		int operacion = recibirInt(socketMemoria, logger); //Recibir header

		switch (operacion) {

		case RESPUESTA: {
			int idScript = recibirInt(socketMemoria, logger);

			int index = encontrarScriptEnLista(idScript, listaEXEC);

			if (idScript == -1 || index == -1) {

				manejoErrorMemoria(memoria->nombre);
				return;

			}

			script* scriptReceptor = list_get(listaEXEC, index);

			int tipoDeRespuesta = recibirInt(socketMemoria, logger);

			int respuesta;

			switch (tipoDeRespuesta) {

			case TODO_BIEN: {

				respuesta = 1;

			}
			case ERROR: {

				respuesta = -1;

			}

				scriptReceptor->resultadoDeEnvio = malloc(sizeof(int));
				memcpy(scriptReceptor->resultadoDeEnvio, &respuesta,
						sizeof(int));

				sem_post(&scriptReceptor->semaforoDelScript);

				continue;

			default: {

				manejoErrorMemoria(memoria->nombre);
				return;
			}

			}

			return; //Esta linea nunca se va a ejecutar, es para que eclipse deje de joder

		}

		case DATO: {

			int idScript = recibirInt(socketMemoria, logger);

			int index = encontrarScriptEnLista(idScript, listaEXEC);

			if (idScript == -1 || index == -1) {
				manejoErrorMemoria(memoria->nombre);
				return;
			}

			script* scriptReceptor = list_get(listaEXEC, index);

			char* datoRecibido = recibirString(socketMemoria, logger);

			if (!strcmp(datoRecibido, " ")) {
				manejoErrorMemoria(memoria->nombre);
				return;
			}

			int respuesta = 1;

			int tamanioString = strlen(datoRecibido) + 1;

			scriptReceptor->resultadoDeEnvio = malloc(
					sizeof(int) + sizeof(int) + tamanioString);

			memcpy(scriptReceptor->resultadoDeEnvio, &respuesta, sizeof(int));

			memcpy(scriptReceptor->resultadoDeEnvio + sizeof(int),
					&tamanioString, sizeof(int));

			memcpy(scriptReceptor->resultadoDeEnvio + sizeof(int) + sizeof(int),
					datoRecibido, strlen(datoRecibido) + 1);

			free(datoRecibido);

			sem_post(&scriptReceptor->semaforoDelScript);

			continue;
		}

		case GOSSIPING: {

			t_list* seeds = recibirSeeds(socketMemoria, logger);

			pthread_t h_gossiping;

			pthread_create(&h_gossiping, NULL, (void *) agregarMemorias, seeds);

			pthread_detach(h_gossiping);

			continue;

		}

		case METADATAS: {

			int idScript = recibirInt(socketMemoria, logger);

			int index = encontrarScriptEnLista(idScript, listaEXEC);

			if (idScript == -1 || index == -1) {
				manejoErrorMemoria(memoria->nombre);
				return;
			}

			script* scriptReceptor = list_get(listaEXEC, index);

			t_list* metadatas = recibirMetadatas(socketMemoria, logger);

			int respuesta = 1;

			scriptReceptor->resultadoDeEnvio = malloc(
					sizeof(int) + sizeof(t_list*));

			memcpy(scriptReceptor->resultadoDeEnvio, &respuesta, sizeof(int));

			memcpy(scriptReceptor->resultadoDeEnvio + sizeof(int), &metadatas,
					sizeof(t_list*)); //Se pasa el puntero

			sem_post(&scriptReceptor->semaforoDelScript);

			continue;
		}

		default: {

			manejoErrorMemoria(memoria->nombre);
			return;
		}

		}

	}
}

