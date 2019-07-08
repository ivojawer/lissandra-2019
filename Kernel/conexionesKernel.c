#include "funcionesKernel.h"

extern t_log* logger;
extern t_list* memorias;
extern sem_t sem_borradoMemoria;
extern t_list* listaEXEC;
extern script* scriptRefreshMetadata;

void conectarseAUnaMemoria(seed* unaSeed) {

	int socketMemoria = conectarseAServidor(unaSeed->ip, unaSeed->puerto);

	if (socketMemoria == -1) {
		return;
	}
	enviarIntConHeader(socketMemoria, KERNEL, HANDSHAKE);

	int headerRespuesta = recibirInt(socketMemoria, logger);

	if (headerRespuesta != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake a la memoria y se devolvio otra cosa.");
		close(socketMemoria);
		return;
	}

	int modulo = recibirInt(socketMemoria, logger);

	if (modulo != MEMORIA) {
		log_error(logger, "Se conecto algo que no es la memoria.");
		close(socketMemoria);
		return;
	}

	int nombreMemoria = recibirInt(socketMemoria, logger);

	if (nombreMemoria == -1) { //Log error ya esta en la funcion
		close(socketMemoria);
		return;
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

	nuevaMemoria->estaViva = 1;
	sem_init(&nuevaMemoria->sem_cambioScriptsEsperando, 0, 1);

	list_create(nuevaMemoria->scriptsEsperando);

	free(unaSeed->ip);

	free(unaSeed);

	sem_wait(&sem_borradoMemoria);

	list_add(memorias, nuevaMemoria);

	sem_post(&sem_borradoMemoria);

	log_info(logger, "Se acaba de conectar la memoria %i",
			nuevaMemoria->nombre);

	comunicacionConMemoria(nuevaMemoria);

}

void manejoErrorMemoria(int nombreMemoria) {
	log_error(logger, "Se desconecto la memoria %i.", nombreMemoria);
	matarMemoria(nombreMemoria);
}

void enviarPeticionesDeGossip()
{
	sem_wait(&sem_borradoMemoria);
	for (int i = 0; i<list_size(memorias);i++)
	{

		memoriaEnLista* unaMemoria = list_get(memorias,i);
		enviarInt(unaMemoria->socket,GOSSIPING);

	}
	sem_post(&sem_borradoMemoria);
}

void agregarMemorias(t_list* seeds) {

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

}

void comunicacionConMemoria(memoriaEnLista* memoria) {
	int socketMemoria = memoria->socket;

	while (1) {
		int operacion = recibirInt(socketMemoria, logger); //Recibir header

		switch (operacion) {

		case RESPUESTA: {
			int idScript = recibirInt(socketMemoria, logger);

			script* unScript = encontrarScriptEnLista(idScript, listaEXEC);

			if (idScript == -1 || unScript->idScript == -1) {

				free(unScript);
				manejoErrorMemoria(memoria->nombre);
				return;

			}
			script* scriptReceptor = unScript;
			int tipoDeRespuesta = recibirInt(socketMemoria, logger);

			int respuesta;

			switch (tipoDeRespuesta) {

			case TODO_BIEN: {

				respuesta = TODO_BIEN;
				break;

			}
			case ERROR: {

				respuesta = ERROR;
				break;
			}

			case MEM_LLENA: {
				respuesta = MEM_LLENA;
				break;
			}

			default: {
				manejoErrorMemoria(memoria->nombre);
				return;
			}

			}

			scriptReceptor->resultadoDeEnvio = malloc(sizeof(int));
			memcpy(scriptReceptor->resultadoDeEnvio, &respuesta, sizeof(int));

			sem_post(&scriptReceptor->semaforoDelScript);

			continue;

		}

		case DATO: {

			int idScript = recibirInt(socketMemoria, logger);

			script* unScript = encontrarScriptEnLista(idScript, listaEXEC);

			if (idScript == -1 || unScript->idScript == -1) {
				free(unScript);
				manejoErrorMemoria(memoria->nombre);
				return;
			}

			script* scriptReceptor = unScript;

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

			if (list_size(seeds) == 0) {
				continue; //Seeds vacias, no hay nada que hacer
			}

			pthread_t h_gossiping;

			pthread_create(&h_gossiping, NULL, (void *) agregarMemorias, seeds);

			pthread_detach(h_gossiping);

			continue;

		}

		case METADATAS: {

			int idScript = recibirInt(socketMemoria, logger);

			script* scriptReceptor;

			if (idScript == 0) {
				scriptReceptor = scriptRefreshMetadata;
			} else {

				script* unScript = encontrarScriptEnLista(idScript, listaEXEC);

				if (idScript == -1 || unScript->idScript == -1) {
					free(unScript);
					manejoErrorMemoria(memoria->nombre);
					return;
				}

				scriptReceptor = unScript;
			}

			t_list* metadatas = recibirMetadatas(socketMemoria, logger);

			metadataTablaLFS* metadataPrueba = list_get(metadatas, 0);

			if (metadataPrueba->consistencia == -1) {
				manejoErrorMemoria(memoria->nombre);
				free(metadataPrueba);
				list_destroy(metadatas);
				return;
			}

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

