#include "funcionesLFS.h"

extern t_log* logger;

void conexiones() {

}

void messageHandler(char* lectura) {
	if (string_is_empty(lectura)) {
		log_info(logger, "No es una request valida, vuelva prontos \n");
		free(lectura);

	}

	char** requestYParametros = string_n_split(lectura, 2, " ");

	int requestEnInt = queRequestEs(requestYParametros[0]);

	if (!esUnaRequestValida(lectura) || requestEnInt == JOURNAL
	|| requestEnInt == ADD || requestEnInt == RUN) { //Si es invalida o es una request que no vale en el LFS

		log_info(logger, "No es una request valida, vuelva prontos \n");

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);
		return;

	}

	if (requestYParametros[1] == NULL) { //Para que no rompa en el string_duplicate de funcionesLFS.c
		requestYParametros[1] = (char *) malloc(sizeof(" "));
		strcpy(requestYParametros[1], " ");
	}
	log_info(logger, "mando a ejecutar una request");

	request* requestParaHilo = malloc(sizeof(request));
	requestParaHilo = crearStructRequest(lectura);

	mandarAEjecutarRequest(requestParaHilo);
}

