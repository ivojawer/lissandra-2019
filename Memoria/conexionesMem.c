#include "conexionesMem.h"

extern t_log* logger;

extern t_list* colaDeRequests;
extern t_list* tablaGossiping;
extern t_list* seedsConocidas;
extern int nombreMemoria;
extern int idScriptKernel;
extern int socketKernel;
extern sem_t sem_recepcionLFS;
extern sem_t sem_LFSconectandose;
extern sem_t sem_gossiping;
extern sem_t requestsDisponibles;
extern sem_t conexionMemoria;
extern char* tablaSelect;
extern char* dirConfig;
extern int puertoServidor;
int socketLFS;

int primeraConexionLFS() {

	t_config* config = config_create(dirConfig);

	char* ipLFS = string_duplicate(config_get_string_value(config, "IP_LFS"));
	int puertoLFS = config_get_int_value(config, "PUERTO_LFS");

	socketLFS = conectarseAServidor(ipLFS, puertoLFS);

	if (socketLFS == -1) {
		config_destroy(config);
		return -1;
	}

	enviarIntConHeader(socketLFS, MEMORIA, HANDSHAKE); //HANDSHAKE-MEMORIA

	int headerRespuesta = recibirInt(socketLFS, logger);

	if (headerRespuesta != HANDSHAKE) {
//		log_error(logger,
//				"Se envio un handshake al LFS y se devolvio otra cosa.");
		config_destroy(config);
		close(socketLFS);
		return -1;
	}

	int moduloConectado = recibirInt(socketLFS, logger);

	if (moduloConectado != LFS) {
//		log_error(logger, "Se conecto algo que no es el LFS.");
		config_destroy(config);
		close(socketLFS);
		return -1;
	}

	int tamanioMaximoValue = recibirInt(socketLFS, logger);

	if (tamanioMaximoValue == -1) {
		config_destroy(config);
		close(socketLFS);
		return -1;
	}

	config_destroy(config);

	return tamanioMaximoValue;

}

void aceptarConexiones() {

	int socketServidor = crearServidor(puertoServidor);

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);

	while (1) {

		int socketMisterioso = accept(socketServidor, (void*) NULL, NULL);

		if (socketMisterioso == -1) {
			continue;
		}

		enviarVariosIntsConHeader(socketMisterioso, listaInts, HANDSHAKE); //HANDSHAKE-MEMORIA-NOMBRE

		int operacion = recibirInt(socketMisterioso, logger);

		if (operacion != HANDSHAKE) {
			log_error(logger, "Se envio un handshake y se devolvio otra cosa.");
			close(socketMisterioso);
			continue;
		}

		int modulo = recibirInt(socketMisterioso, logger);

		switch (modulo) {
		case KERNEL: {
			log_info(logger, "Se conecto el Kernel.");
			socketKernel = socketMisterioso;
			pthread_t h_conexionKernel;
			pthread_create(&h_conexionKernel, NULL,
					(void *) comunicacionConKernel,
					NULL);
			pthread_detach(h_conexionKernel);

			continue;
		}

		case MEMORIA: {
			sem_wait(&conexionMemoria);

			int nombre = recibirInt(socketMisterioso, logger);

			int puertoMemoriaConectandose = recibirInt(socketMisterioso,
					logger);

			if (nombre == nombreMemoria) //Si se conecto la memoria misma
					{
				sem_post(&conexionMemoria);
				continue; //El hilo que envio se encarga de agregarse a la tabla
			}

			sem_wait(&sem_gossiping);
			if (nombre == -1 || memoriaYaEstaConectada(nombre)) { //Si la memoria ya esta conectada, ya se conoce su seed
//				close(socketMisterioso); TODO: ???
				sem_post(&sem_gossiping);
				sem_post(&conexionMemoria);
				continue;
			}

			seed* seedMemoria = malloc(sizeof(seed));
			seedMemoria->ip = ipDelCliente(socketMisterioso);
			seedMemoria->puerto = puertoMemoriaConectandose;

			memoriaGossip* nuevaMemoriaConectada = malloc(
					sizeof(memoriaGossip));
			nuevaMemoriaConectada->nombre = nombre;
			nuevaMemoriaConectada->elSocket = socketMisterioso;
			nuevaMemoriaConectada->laSeed = seedMemoria;

			enviarSeedsConectadas(nuevaMemoriaConectada, GOSSIPING);

			int operacion = recibirInt(socketMisterioso, logger);

			t_list* seedsRecibidas = recibirSeeds(socketMisterioso, logger);

			if (list_size(seedsRecibidas) != 0) {

				seed* seedPrueba = list_get(seedsRecibidas, 0);
				if (seedPrueba->puerto == -1 || operacion != GOSSIPING) {

					close(socketMisterioso);
					free(seedMemoria->ip);
					free(seedMemoria);
					free(nuevaMemoriaConectada);
					free(seedPrueba);
					list_destroy(seedsRecibidas);
					log_error(logger, "Fallo la conexion a la memoria %i.",
							nombre);
					sem_post(&sem_gossiping);
					sem_post(&conexionMemoria);
					continue;
				}
				agregarNuevasSeeds(seedsRecibidas);

			}

			if (!seedExiste(seedMemoria)) {
				list_add(seedsConocidas, seedMemoria);
			}

			list_add(tablaGossiping, nuevaMemoriaConectada);

			log_info(logger, "Se conecto la memoria %i.", nombre);

			sem_post(&conexionMemoria);
//			tratarDeConectarseASeeds();

			sem_post(&sem_gossiping);

			pthread_t h_comunicacionConMemoria;
			pthread_create(&h_comunicacionConMemoria, NULL,
					(void *) comunicacionConMemoria, nuevaMemoriaConectada);
			pthread_detach(h_comunicacionConMemoria);

			continue;
		}

		default: {
			log_error(logger, "Se conecto indebidamente algo.");
			close(socketMisterioso);
			continue;
		}

		}

	}

}

void conectarseAOtraMemoria(seed* laSeed) { //Esto es secuencial al hilo de gossiping, no es necesario sincronizar

	sem_wait(&conexionMemoria);

	int socketMemoria = conectarseAServidor(laSeed->ip, laSeed->puerto);

	if (socketMemoria == -1) {
		sem_post(&conexionMemoria);
		return;
	}

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);
	list_add(listaInts, &puertoServidor);

	enviarVariosIntsConHeader(socketMemoria, listaInts, HANDSHAKE); //HANDSHAKE-MEMORIA-NOMBRE

	list_remove(listaInts, 0);
	list_remove(listaInts, 0);
	list_destroy(listaInts);

	int operacion = recibirInt(socketMemoria, logger);

	if (operacion != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake a una memoria y se devolvio otra cosa.");
		close(socketMemoria);
		sem_post(&conexionMemoria);
		return;
	}

	int modulo = recibirInt(socketMemoria, logger);

	if (modulo != MEMORIA) {
		log_error(logger,
				"Se conecto indebidamente algo que no es una memoria.");
		close(socketMemoria);
		sem_post(&conexionMemoria);
		return;
	}

	int nombre = recibirInt(socketMemoria, logger);

	if (nombre == -1) {
		log_error(logger, "Se recibio algo indebido de una memoria.");
		close(socketMemoria);
		sem_post(&conexionMemoria);
		return;
	}

	if (memoriaYaEstaConectada(nombre)) {

		memoriaGossip* memoriaYaConectada = structMemoriaDadoElNombre(nombre);

		if (memoriaYaConectada->laSeed == NULL) { //Si la memoria no tenia seed, se le agrega
			memoriaYaConectada->laSeed = laSeed;
		}

		free(laSeed->ip);
		free(laSeed);

		sem_post(&conexionMemoria);

		return; //Ya se hizo gossiping

	}

	memoriaGossip* nuevaMemoriaConectada = malloc(sizeof(memoriaGossip));
	nuevaMemoriaConectada->nombre = nombre;
	nuevaMemoriaConectada->elSocket = socketMemoria;
	nuevaMemoriaConectada->laSeed = laSeed;

	if (nombre != nombreMemoria) {

		enviarSeedsConectadas(nuevaMemoriaConectada, GOSSIPING);

		int operacion = recibirInt(socketMemoria, logger);

		t_list* seedsRecibidas = recibirSeeds(socketMemoria, logger);

		if (list_size(seedsRecibidas) != 0) {

			seed* seedPrueba = list_get(seedsRecibidas, 0);
			if (seedPrueba->puerto == -1 || operacion != GOSSIPING) {

				close(socketMemoria);
				free(nuevaMemoriaConectada);
				free(seedPrueba);
				list_destroy(seedsRecibidas);
				log_error(logger, "Fallo la conexion a la memoria %i.", nombre);
				free(laSeed->ip);
				free(laSeed);
				return;
			}
			agregarNuevasSeeds(seedsRecibidas);

		}
		list_add(tablaGossiping, nuevaMemoriaConectada);

		log_info(logger, "Se conecto a la memoria %i.", nombre);

		sem_post(&conexionMemoria);
//		tratarDeConectarseASeeds();

		pthread_t h_comunicacionConMemoria;
		pthread_create(&h_comunicacionConMemoria, NULL,
				(void *) comunicacionConMemoria, nuevaMemoriaConectada);
		pthread_detach(h_comunicacionConMemoria);

	} else {
		sem_post(&conexionMemoria);
		list_add(tablaGossiping, nuevaMemoriaConectada);
	}

}

void comunicacionConMemoria(memoriaGossip* memoria) {

	int socketMemoria = memoria->elSocket;
	while (1) {
		int operacion = recibirInt(socketMemoria, logger);
		log_info(logger, "%i", operacion);

		switch (operacion) {
		case GOSSIPING: {
			t_list* seedsRecibidas = recibirSeeds(socketMemoria, logger);
			sem_wait(&sem_gossiping);

			if (list_size(seedsRecibidas) != 0) {

				seed* seedPrueba = list_get(seedsRecibidas, 0);
				if (seedPrueba->puerto == -1) {

					sacarMemoriaDeTablaGossip(memoria);

					free(seedPrueba);
					list_destroy(seedsRecibidas);
					return;
				}
				agregarNuevasSeeds(seedsRecibidas);
//				tratarDeConectarseASeeds();

			} //Si list_size == 0 no se hace nada
			list_destroy(seedsRecibidas);
			enviarSeedsConectadas(memoria, RESPUESTA);
			sem_post(&sem_gossiping);

			continue;
		}
		case RESPUESTA: {
			t_list* seedsRecibidas = recibirSeeds(socketMemoria, logger);

			if (list_size(seedsRecibidas) != 0) {

				seed* seedPrueba = list_get(seedsRecibidas, 0);
				if (seedPrueba->puerto == -1) {
					sem_wait(&sem_gossiping);
					sacarMemoriaDeTablaGossip(memoria);
					sem_post(&sem_gossiping);

					free(seedPrueba);
					list_destroy(seedsRecibidas);
					return;
				}
			}

			sem_wait(&sem_gossiping);
			agregarNuevasSeeds(seedsRecibidas);
//			tratarDeConectarseASeeds();
			sem_post(&sem_gossiping);
			continue;
		}

		default: {
			sem_wait(&sem_gossiping);
			sacarMemoriaDeTablaGossip(memoria);
			sem_post(&sem_gossiping);
			return;
		}
		}
	}
}

void comunicacionConKernel() {
	while (1) {

		int operacion = recibirInt(socketKernel, logger);

		switch (operacion) {

		case REQUEST: {

			int id = recibirInt(socketKernel, logger);

			if (id == -1) {
				manejoErrorKernel();
				return;
			}

			request* unaRequest = recibirRequest(socketKernel, logger);

			if (unaRequest->requestEnInt == -1) {
				manejoErrorKernel();
				return;
			}

			requestConID* requestParaHilo = malloc(sizeof(requestConID));

			requestParaHilo->laRequest = unaRequest;
			requestParaHilo->idKernel = id;

			list_add(colaDeRequests, requestParaHilo);
			sem_post(&requestsDisponibles);

			break;

		}
		case GOSSIPING: {

			int seedEstaConectada(seed* unaSeed) {

				return !seedNoEstaConectada(unaSeed);
			}

			log_info(logger, "Se recibio la peticion de GOSSIPING del kernel.");

			sem_wait(&sem_gossiping);
			t_list* seedsConectadas = list_filter(seedsConocidas,
					(void*) seedEstaConectada);
			enviarSeedsConHeader(socketKernel, seedsConectadas, GOSSIPING);
			sem_post(&sem_gossiping);
			continue;
		}
		case OP_JOURNAL: {
			request* unaRequest = malloc(sizeof(request));
			unaRequest->requestEnInt = JOURNAL;
			unaRequest->parametros = NULL;

			requestConID* requestParaHilo = malloc(sizeof(requestConID));

			requestParaHilo->laRequest = unaRequest;
			requestParaHilo->idKernel = 0;

			list_add(colaDeRequests, requestParaHilo);
			sem_post(&requestsDisponibles);

			continue;
		}

		default: {
			manejoErrorKernel();
			return;
		}

		}
	}

}

char* ponerComillas(char* string) {
	printf("hola\n");
	char* cadenaConComillas = malloc(
			(string_length(string) + 2) * sizeof(char));
	*cadenaConComillas = '"';
	strcpy(&cadenaConComillas[1], string);
	cadenaConComillas[string_length(string) + 1] = '"';
	printf("con comillas:%s\n", cadenaConComillas);
	return cadenaConComillas;
}

void enviarRegistroComoInsert(registro* registroAEnviar) {

//	int lengthKey = snprintf( NULL, 0, "%d", registroAEnviar->key);
//	char* keyEnString = malloc(lengthKey + 1);
//	snprintf(keyEnString, lengthKey + 1, "%d", registroAEnviar->key);

	int lengthTS = snprintf( NULL, 0, "%d", registroAEnviar->timestamp); //Cuando sea la hora habra que cambiar esto (probablemente por %llu)
	char* tsEnString = malloc(lengthTS + 1);
	snprintf(tsEnString, lengthTS + 1, "%d", registroAEnviar->timestamp);

	request* insertAEnviar = malloc(sizeof(request));
	insertAEnviar->requestEnInt = INSERT;

	char* parametros = string_new();

	char* tabla = "TablaRandom"; //TODO: No se especifica la tabla

	string_append(&parametros, tabla);
	string_append(&parametros, " ");
	string_append(&parametros, string_itoa(registroAEnviar->key));
	string_append(&parametros, " \"");
	string_append(&parametros, registroAEnviar->value);
	string_append(&parametros, "\" ");
	string_append(&parametros, tsEnString);
	free(tsEnString);

	insertAEnviar->parametros = parametros;

	requestConID* requestAEjecutar = malloc(sizeof(requestConID));

	requestAEjecutar->laRequest = insertAEnviar;
	requestAEjecutar->idKernel = 0;

	list_add(colaDeRequests, requestAEjecutar);
	sem_post(&requestsDisponibles);

}

int memoriaYaEstaConectada(int nombreMemoria) { //Sincronizar por afuera

	int esLaMismaMemoria(memoriaGossip* otraMemoria) {
		if (otraMemoria->nombre == nombreMemoria) {
			return 1;
		}
		return 0;
	}

	int resultado = list_any_satisfy(tablaGossiping, (void*) esLaMismaMemoria);

	return resultado;
}

memoriaGossip* structMemoriaDadoElNombre(int nombreMemoria) { //Sincronizar por afuera

	int esLaMismaMemoria(memoriaGossip* otraMemoria) {
		if (otraMemoria->nombre == nombreMemoria) {
			return 1;
		}
		return 0;
	}

	return list_find(tablaGossiping, (void*) esLaMismaMemoria);
}

void enviarRespuestaAlKernel(int id, int respuesta) {
	t_list* intsAEnviar = list_create();

	list_add(intsAEnviar, &id);
	list_add(intsAEnviar, &respuesta);

	enviarVariosIntsConHeader(socketKernel, intsAEnviar, RESPUESTA);
}
void manejoErrorLFS() {

	sem_wait(&sem_LFSconectandose);
	if (socketLFS == -1) {
		sem_post(&sem_LFSconectandose);
		return;
	}
	sem_post(&sem_LFSconectandose);

	log_error(logger, "Se desconecto el LFS.");
	close(socketLFS);
	socketLFS = -1;

	pthread_t h_reconexionLFS;
	pthread_create(&h_reconexionLFS, NULL, (void *) reconexionLFS, NULL);
	pthread_detach(h_reconexionLFS);

}

void manejoErrorKernel() {
	log_error(logger,
			"Se recibio del kernel algo incorrecto, se va a cerrar la conexion.");
	if (socketKernel != -1) {
		close(socketKernel);
	}

}

void manejarRespuestaLFS() {

	while (1) {
		int operacion = recibirInt(socketLFS, logger);

		switch (operacion) {
		case DATO: {
			char* dato = recibirString(socketLFS, logger);

			if (!strcmp(dato, " ")) {

				manejoErrorLFS();
				log_error(logger, "No se pudo ejecutar el request");
				if (idScriptKernel) {
					enviarRespuestaAlKernel(idScriptKernel, ERROR);
					log_info(logger, "Enviando ERROR al kernel");
				}

				free(dato);
				sem_post(&sem_recepcionLFS);
				return;
			}

			log_info(logger, "Resultado: %s", dato);
			if (idScriptKernel) {

				log_info(logger, "Enviando el resultado al kernel");
				enviarStringConHeaderEId(socketKernel, dato, DATO,
						idScriptKernel);
			}
			sem_post(&sem_recepcionLFS);
			continue;
		}
		case RESPUESTA: {
			int respuesta = recibirInt(socketLFS, logger);

			if (respuesta == -1 || respuesta == 0) {

				manejoErrorLFS();
				log_error(logger, "No se pudo ejecutar el request");

				if (idScriptKernel) {
					log_info(logger, "Enviando ERROR al kernel");
					enviarRespuestaAlKernel(idScriptKernel, ERROR);
				}

				sem_post(&sem_recepcionLFS);

				return;
			}
			if (respuesta == TODO_BIEN) {
				log_info(logger, "El LFS pudo ejecutar la request");
			} else if (respuesta == NO_EXISTE) {
				log_info(logger, "El LFS no tiene el dato");
			} else if (respuesta == ERROR) {
				log_info(logger, "El LFS no pudo ejecutar la request");
			} else if (respuesta == TABLA_NO_EXISTE) {
				log_info(logger, "El LFS no tiene la tabla");
			}
			if (idScriptKernel) {
				log_info(logger, "Enviando el resultado al kernel");
				enviarRespuestaAlKernel(idScriptKernel, respuesta);
			}
			sem_post(&sem_recepcionLFS);
			continue;
		}
		case REGISTRO: {
			registro* registroRecibido = recibirRegistro(socketLFS, logger);
			if (!strcmp(registroRecibido->value, " ")) {

				manejoErrorLFS();
				log_error(logger, "No se pudo ejecutar el request");

				if (idScriptKernel) {
					log_info(logger, "Enviando ERROR al kernel");
					enviarRespuestaAlKernel(idScriptKernel, ERROR);
				}

				sem_post(&sem_recepcionLFS);

				return;
			}

			log_info(logger, "Resultado: %s", registroRecibido->value);

			if (idScriptKernel) {

				log_info(logger, "Enviando el resultado al kernel");
				enviarStringConHeaderEId(socketKernel, registroRecibido->value,
				DATO, idScriptKernel);
			}

			insertInterno(registroRecibido->key, registroRecibido->value,
					tablaSelect, registroRecibido->timestamp); //TODO: Marca de que esto puede no insertar si la memoria esta llena
			sem_post(&sem_recepcionLFS);
			continue;
		}
		case METADATAS: {
			t_list* metadatas = recibirMetadatas(socketLFS, logger);

			if (list_size(metadatas) != 0) {
				metadataTablaLFS* metadataPrueba = list_get(metadatas, 0);

				if (metadataPrueba->consistencia == -1) {
					enviarRespuestaAlKernel(idScriptKernel, ERROR);
					free(metadataPrueba);
					list_remove(metadatas, 0);
					list_destroy(metadatas);
					manejoErrorLFS();
					log_error(logger, "No se pudo ejecutar el request");

					if (idScriptKernel) {
						log_info(logger, "Enviando ERROR al kernel");
						enviarRespuestaAlKernel(idScriptKernel, ERROR);
					}

					sem_post(&sem_recepcionLFS);
					return;
				}
			}

			describirMetadatas(metadatas);

			if (idScriptKernel) {

				if (list_size(metadatas) == 1) //HAY QUE TENER ESA BUENA COHERENCIA DE SINGULAR/PLURAL
						{
					log_info(logger, "Enviando la metadata al kernel");
				} else {
					log_info(logger, "Enviando las metadatas al kernel");
				}

				enviarMetadatasConHeaderEId(socketKernel, metadatas,
				METADATAS, idScriptKernel);
			}

			liberarListaMetadatas(metadatas);

			sem_post(&sem_recepcionLFS);
			continue;
		}
		default: {

			manejoErrorLFS();
			log_error(logger, "No se pudo ejecutar el request");

			if (idScriptKernel != -1) //Si habia un script en ejecucion
					{
				log_info(logger, "Enviando ERROR al kernel");
				enviarRespuestaAlKernel(idScriptKernel, ERROR);
				sem_post(&sem_recepcionLFS);
			}

			return;
		}
		}
	}

}
