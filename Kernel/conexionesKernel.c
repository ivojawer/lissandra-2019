#include "funcionesKernel.h"

extern t_log* logger;
extern t_list* memorias;
extern t_list* listaEXEC;
extern t_list* seedsMemorias;
extern t_list* seedsSiendoCreadas;
extern sem_t sem_borradoMemoria;
extern sem_t sem_gossiping;
extern sem_t sem_seedSiendoCreada;
extern sem_t sem_gossiping;
extern script* scriptRefreshMetadata;

void conectarseAUnaMemoria(seed* unaSeed) {
	int socketMemoria = conectarseAServidor(unaSeed->ip, unaSeed->puerto);

	if (socketMemoria == -1) {
		sacarSeedDeMemoriasEnCreacion(unaSeed);
		return;
	}
	enviarIntConHeader(socketMemoria, KERNEL, HANDSHAKE);

	int headerRespuesta = recibirInt(socketMemoria, logger);

	if (headerRespuesta != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake a la memoria y se devolvio otra cosa.");
		sacarSeedDeMemoriasEnCreacion(unaSeed);
		close(socketMemoria);
		return;
	}

	int modulo = recibirInt(socketMemoria, logger);

	if (modulo != MEMORIA) {
		log_error(logger, "Se conecto algo que no es la memoria.");
		sacarSeedDeMemoriasEnCreacion(unaSeed);
		close(socketMemoria);
		return;
	}

	int nombreMemoria = recibirInt(socketMemoria, logger);

	if (nombreMemoria == -1) { //Log error ya esta en la funcion
		sacarSeedDeMemoriasEnCreacion(unaSeed);
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

	nuevaMemoria->ip = unaSeed->ip;
	nuevaMemoria->puerto = unaSeed->puerto;

	nuevaMemoria->estaViva = 1;
	sem_init(&nuevaMemoria->sem_cambioScriptsEsperando, 0, 1);

	nuevaMemoria->scriptsEsperando = list_create();

	nuevaMemoria->estadisticas.insertsCompletos = 0;
	nuevaMemoria->estadisticas.insertsFallidos = 0;
	nuevaMemoria->estadisticas.selectsCompletos = 0;
	nuevaMemoria->estadisticas.selectsFallidos = 0;
	nuevaMemoria->estadisticas.operacionesTotalesEnMemoria = 0;

	sem_wait(&sem_borradoMemoria);

	list_add(memorias, nuevaMemoria);

	sem_post(&sem_borradoMemoria);

	sacarSeedDeMemoriasEnCreacion(unaSeed);

	log_info(logger, "Se conecto la memoria %i", nuevaMemoria->nombre);


	comunicacionConMemoria(nuevaMemoria);

}

void manejoErrorMemoria(int nombreMemoria) {
	log_error(logger, "Se desconecto la memoria %i.", nombreMemoria);
	matarMemoria(nombreMemoria);
}

void enviarPeticionesDeGossip() {
	sem_wait(&sem_borradoMemoria);
	for (int i = 0; i < list_size(memorias); i++) {

		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->estaViva) {
			enviarInt(unaMemoria->socket, GOSSIPING);
		}
	}
	sem_post(&sem_borradoMemoria);
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

		list_add(seedsMemorias, unaSeed);
		sem_wait(&sem_seedSiendoCreada);
		list_add(seedsSiendoCreadas, unaSeed);
		sem_post(&sem_seedSiendoCreada);

		pthread_t h_nuevaConexion;

		pthread_create(&h_nuevaConexion, NULL, (void *) conectarseAUnaMemoria,
				unaSeed);

		pthread_detach(h_nuevaConexion);

	}

	list_destroy(seeds);
	sem_post(&sem_gossiping);

}

void conectarseASeedsDesconectadas() {

	sem_wait(&sem_gossiping);

	for (int i = 0; i < list_size(seedsMemorias); i++) {
		seed* unaSeed = list_get(seedsMemorias, i);

		if (!seedYaEstaConectada(unaSeed)
				&& !memoriaEstaSiendoCreada(unaSeed)) {

			sem_wait(&sem_seedSiendoCreada);
			list_add(seedsSiendoCreadas, unaSeed);
			sem_post(&sem_seedSiendoCreada);

			pthread_t h_nuevaConexion;

			pthread_create(&h_nuevaConexion, NULL,
					(void *) conectarseAUnaMemoria, unaSeed);

			pthread_detach(h_nuevaConexion);
		}
	}
	sem_post(&sem_gossiping);
}

void comunicacionConMemoria(memoriaEnLista* memoria) {
	int socketMemoria = memoria->socket;

	while (1) {
		int operacion = recibirInt(socketMemoria, logger); //Header

		switch (operacion) {

		case RESPUESTA: {
			int idScript = recibirInt(socketMemoria, logger);

			script* unScript;

			if (idScript == 1) {
				unScript = scriptRefreshMetadata;

			} else {

				unScript = encontrarScriptEnLista(idScript, listaEXEC);

				if (idScript == -1 || unScript->idScript == -1) {

					free(unScript);
					manejoErrorMemoria(memoria->nombre);
					return;

				}

			}

			script* scriptReceptor = unScript;
			int tipoDeRespuesta = recibirInt(socketMemoria, logger);

			int respuesta;

			switch (tipoDeRespuesta) { //Juro que esto tiene algun sentido, pero me lo olvide

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
			case NO_EXISTE:{
				respuesta = NO_EXISTE;
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

			int respuesta = TODO_BIEN;

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

			log_info(logger, "Se recibio el gossiping de la memoria",
					memoria->nombre);

			t_list* seeds = recibirSeeds(socketMemoria, logger);

			if (list_size(seeds) == 0) {
				continue; //Seeds vacias, no hay nada que hacer
			}

			seed* seedPrueba = list_get(seeds, 0);

			if (seedPrueba->puerto == -1) {
				manejoErrorMemoria(memoria->nombre);
				list_remove(seeds, 0);
				list_destroy(seeds);
				free(seedPrueba);
				return;
			}

			pthread_t h_gossiping;

			pthread_create(&h_gossiping, NULL, (void *) agregarMemorias, seeds);

			pthread_detach(h_gossiping);

			continue;

		}

		case METADATAS: {

			int idScript = recibirInt(socketMemoria, logger);

			script* scriptReceptor;

			if (idScript == 1) {
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

			if (list_size(metadatas) != 0) {
				metadataTablaLFS* metadataPrueba = list_get(metadatas, 0);

				if (metadataPrueba->consistencia == -1) {
					manejoErrorMemoria(memoria->nombre);
					free(metadataPrueba);
					list_destroy(metadatas);
					return;
				}

			}

			int respuesta = TODO_BIEN;

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

