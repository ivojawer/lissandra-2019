#include "funcionesLFS.h"

void consola() {
	while (1){
		printf(">");
		char *lectura = readline("");
		add_history(lectura);

		if (string_is_empty(lectura)){
			printf("No es una request valida, vuelva prontos \n");
			continue;
		}

		char** requestYParametros = string_n_split(lectura, 2, " ");
		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!esUnaRequestValida(lectura) || requestEnInt == JOURNAL || requestEnInt == ADD
			|| requestEnInt == RUN || requestEnInt == METRICS){ //Si es invalida o es una request que no vale en el LFS
			printf("No es una request valida, vuelva prontos \n");
			liberarArrayDeStrings(requestYParametros);
			continue;
		}
		request* requestParaHilo = crearStructRequest(lectura);
		mandarAEjecutarRequest(requestParaHilo, -1); //Esto podria ser un hilo?
		liberarArrayDeStrings(requestYParametros);
	}
}
