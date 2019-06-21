#include "conexionesMem.h"

extern t_log* logger;
extern sem_t requestsDisponibles;
extern t_list* colaDeRequests;
extern t_list* tablaGossiping;
extern int nombreMemoria;
extern sem_t sem_gossiping;
int socketKernel;
int socketLFS;

int primeraConexionLFS() {

	t_config* config = config_create(DIRCONFIG);

	char* ipLFS = string_duplicate(config_get_string_value(config, "IP_LFS"));
	int puertoLFS = config_get_int_value(config, "PUERTO_LFS");

	socketLFS = conectarseAServidor(ipLFS, puertoLFS);

	enviarIntConHeader(socketLFS, MEMORIA, HANDSHAKE); //HANDSHAKE-MEMORIA

	int headerRespuesta = recibirInt(socketLFS, logger);

	if (headerRespuesta != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake al LFS y se devolvio otra cosa.");
		close(socketLFS);
		return -1;
	}

	int moduloConectado = recibirInt(socketLFS, logger);

	if (moduloConectado != LFS) {
		log_error(logger, "Se conecto algo que no es el LFS.");
		close(socketLFS);
		return -1;
	}

	int tamanioMaximoValue = recibirInt(socketLFS, logger);

	config_destroy(config);

	return tamanioMaximoValue;

}

void aceptarConexiones() {
	t_config* config = config_create(DIRCONFIG);

	int puertoServidor = config_get_int_value(config, "PUERTO_ESCUCHA");

	int socketServidor = crearServidor(puertoServidor);

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);

	config_destroy(config);

	while (1) {

		int socketMisterioso = accept(socketServidor, (void*) NULL, NULL);

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
			int nombre = recibirInt(socketMisterioso, logger);

			sem_wait(&sem_gossiping);
			if (nombre == -1 || memoriaYaEstaConectada(nombre)) { //Si la memoria ya esta conectada, ya se conoce su seed
				close(socketMisterioso);
				sem_post(&sem_gossiping);
				continue;
			}
			sem_post(&sem_gossiping);

			memoriaGossip* nuevaMemoriaConectada = malloc(
					sizeof(memoriaGossip));
			nuevaMemoriaConectada->nombre = nombre;
			nuevaMemoriaConectada->elSocket = socketMisterioso;
			nuevaMemoriaConectada->laSeed = NULL;

			sem_wait(&sem_gossiping);

			int respuesta = enviarYRecibirSeeds(nuevaMemoriaConectada);

			if (respuesta == -1) {
				log_error(logger, "Fallo la conexion a la memoria %i.", nombre);
				close(socketMisterioso);
				free(nuevaMemoriaConectada);
				continue;
			}

			nuevaMemoriaConectada->laSeed = NULL;

			list_add(tablaGossiping, nuevaMemoriaConectada);

			sem_post(&sem_gossiping);

			log_info(logger, "Se conecto la memoria %i.", nombre);

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

	int socketMemoria = conectarseAServidor(laSeed->ip, laSeed->puerto);

	if (socketMemoria == -1) {
		return;
	}

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);

	enviarVariosIntsConHeader(socketKernel, listaInts, HANDSHAKE); //HANDSHAKE-MEMORIA-NOMBRE

	int operacion = recibirInt(socketKernel, logger);

	if (operacion != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake a una memoria y se devolvio otra cosa.");
		list_destroy(listaInts);
		close(socketMemoria);
		return;
	}

	int modulo = recibirInt(socketKernel, logger);

	if (modulo != MEMORIA) {
		log_error(logger,
				"Se conecto indebidamente algo que no es una memoria.");
		list_destroy(listaInts);
		close(socketMemoria);
		return;
	}

	int nombre = recibirInt(socketKernel, logger);

	if (nombre == -1) {
		log_error(logger, "Se recibio algo indebido de una memoria.");
		list_destroy(listaInts);
		close(socketMemoria);
		return;
	}

	sem_wait(&sem_gossiping);
	if (memoriaYaEstaConectada(nombre)) {

		int index = posicionMemoriaEnLista(nombre);

		memoriaGossip* memoriaYaConectada = list_get(tablaGossiping,index);
		memoriaYaConectada->laSeed = laSeed;
		sem_post(&sem_gossiping);
		return;

	}

	memoriaGossip* nuevaMemoriaConectada = malloc(sizeof(memoriaGossip));
	nuevaMemoriaConectada->nombre = nombre;
	nuevaMemoriaConectada->elSocket = socketMemoria;
	nuevaMemoriaConectada->laSeed = laSeed;

	int respuesta = enviarYRecibirSeeds(nuevaMemoriaConectada);

	if (respuesta == -1) {
		log_error(logger, "Fallo la conexion a la memoria %i.", nombre);
		close(socketMemoria);
		free(nuevaMemoriaConectada);
		free(laSeed->ip);
		free(laSeed);
		return;
	}

	list_add(tablaGossiping, nuevaMemoriaConectada);

	sem_post(&sem_gossiping);

	log_info(logger, "Se conecto a la memoria %i.", nombre);

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
//			enviarSeedsConHeader(socketKernel, seeds, GOSSIPING);
			continue;
		}
		case JOURNAL: {
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

void enviarInsert(registro reg) {

	int lengthKey = snprintf( NULL, 0, "%d", reg.key);
	char* keyEnString = malloc(lengthKey + 1);
	snprintf(keyEnString, lengthKey + 1, "%d", reg.key);

	int lengthTS = snprintf( NULL, 0, "%d", reg.timestamp);
	char* tsEnString = malloc(lengthTS + 1);
	snprintf(tsEnString, lengthTS + 1, "%d", reg.key);

	char* parametros = "";
	string_append(parametros, keyEnString);
	string_append_with_format(parametros, " ", reg.value);
	string_append_with_format(parametros, " ", tsEnString);
	printf("insert a enviar:%s", parametros);
	free(keyEnString);
	free(tsEnString);

	enviarStringConHeader(socketLFS, parametros, INSERT);

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

int posicionMemoriaEnLista(int nombreMemoria) { //Sincronizar por afuera

	int esLaMismaMemoria(memoriaGossip* otraMemoria) {
		if (otraMemoria->nombre == nombreMemoria) {
			return 1;
		}
		return 0;
	}

	int* resultado = list_find(tablaGossiping, (void*) esLaMismaMemoria);


	return *resultado;
}

void enviarRespuestaAlKernel(int id, int respuesta) {
	t_list* intsAEnviar = list_create();

	list_add(intsAEnviar, &id);
	list_add(intsAEnviar, &respuesta);

	enviarVariosIntsConHeader(socketKernel, intsAEnviar, RESPUESTA);
}
void manejoErrorLFS() {
	log_error(logger,
			"Se recibio del LFS algo incorrecto, se va a cerrar la conexion.");
	close(socketLFS);
}

void manejoErrorKernel() {
	log_error(logger,
			"Se recibio del kernel algo incorrecto, se va a cerrar la conexion.");
	close(socketKernel);
}

