#include "conexionesMem.h"

extern t_log* logger;
extern t_list* seeds;
extern sem_t requestsDisponibles;
extern t_list* colaDeRequests;
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

void primeraConexionKernel() {
	t_config* config = config_create(DIRCONFIG);

	int puertoServidor = config_get_int_value(config, "PUERTO_ESCUCHA");

	int socketServidor = crearServidor(puertoServidor);

	socketKernel = accept(socketServidor, (void*) NULL, NULL);

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	int nombreMemoria = config_get_int_value(config, "MEMORY_NUMBER");

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);

	enviarVariosIntsConHeader(socketKernel, listaInts, HANDSHAKE); //HANDSHAKE-MEMORIA-NOMBRE

	int operacion = recibirInt(socketKernel, logger);

	if (operacion != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake al kernel y se devolvio otra cosa.");
		return;
	}

	int modulo = recibirInt(socketKernel, logger);

	if (modulo != KERNEL) {
		log_error(logger, "Se conecto algo que no es el kernel.");
		return;
	}

	config_destroy(config);
	list_destroy(listaInts);

	comunicacionConKernel();

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
			enviarSeedsConHeader(socketKernel, seeds, GOSSIPING);
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

void enviarRespuestaAlKernel(int id, int respuesta) {
	t_list* intsAEnviar = list_create();

	list_add(intsAEnviar, &id);
	list_add(intsAEnviar, &respuesta);

	enviarVariosIntsConHeader(socketKernel, intsAEnviar,
	RESPUESTA);
}

void manejoErrorKernel() {
	log_error(logger,
			"Se recibio del kernel algo incorrecto, se va a cerrar la conexion.");
	close(socketKernel);
}

void manejoErrorLFS() {
	log_error(logger,
			"Se recibio del LFS algo incorrecto, se va a cerrar la conexion.");
	close(socketLFS);
}
