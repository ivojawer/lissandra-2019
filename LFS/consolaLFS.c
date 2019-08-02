#include "funcionesLFS.h"
extern sem_t sem_refreshConfig;
extern int retardo; //en milisegundos
extern int tiempoDump; //en milisegundos

void consola() {
	while (1) {
		char *lectura = readline(">");
		add_history(lectura);

		if (string_is_empty(lectura)) {
			printf("No es una request valida, vuelva prontos \n");
			continue;
		}

		char** requestYParametros = string_n_split(lectura, 2, " ");
		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!strcmp(requestYParametros[0], "SLEEP")
				&& esUnNumero(requestYParametros[1])) {
			sem_wait(&sem_refreshConfig);
			retardo = atoi(requestYParametros[1]);
			sem_post(&sem_refreshConfig);
			continue;

		}
		if (!strcmp(requestYParametros[0], "SLEEP_DUMP")
				&& esUnNumero(requestYParametros[1])) {
			sem_wait(&sem_refreshConfig);
			tiempoDump = atoi(requestYParametros[1]);
			sem_post(&sem_refreshConfig);
			continue;
		}

		if ((!esUnaRequestValida(lectura)
				&& strcmp(requestYParametros[0], "CONFIG_RETARDO") != 0
				&& strcmp(requestYParametros[0], "CONFIG_DUMP") != 0)
				|| requestEnInt == JOURNAL || requestEnInt == ADD
				|| requestEnInt == RUN || requestEnInt == METRICS) { //Si es invalida o es una request que no vale en el LFS
			printf("No es una request valida, vuelva prontos \n");
			liberarArrayDeStrings(requestYParametros);
			continue;
		}
		request* requestParaHilo;
		if (!esUnaRequestValida(lectura)) {
			requestParaHilo = crearStructRequestInternaLFS(lectura);
		} else {
			requestParaHilo = crearStructRequest(lectura);
		}
		mandarAEjecutarRequest(requestParaHilo, -1); //Esto podria ser un hilo?
		liberarArrayDeStrings(requestYParametros);
	}
}

request* crearStructRequestInternaLFS(char* requestEnString) {
	char** requestYParametros = string_n_split(requestEnString, 2, " ");

	int requestInt = queRequestEsInterno(requestYParametros[0]);

	request* requestNuevo = malloc(sizeof(request));
	requestNuevo->requestEnInt = requestInt;

	if (requestYParametros[1] == NULL) {
		char* parametrosRequest = string_duplicate(" ");
		requestNuevo->parametros = parametrosRequest;
	} else {
		char* parametrosRequest = string_duplicate(requestYParametros[1]);
		requestNuevo->parametros = parametrosRequest;
	}

	liberarArrayDeStrings(requestYParametros);

	return requestNuevo;
}

int queRequestEsInterno(char* palabra) {

	if (strcmp(palabra, "CONFIG_RETARDO") == 0) {
		return CONFIG_RETARDO;
	} else if (strcmp(palabra, "CONFIG_DUMP") == 0) {
		return CONFIG_DUMP;
	}
	return -1;
}
