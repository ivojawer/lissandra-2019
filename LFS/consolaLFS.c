#include "funcionesLFS.h"

void consola() {

	while (1)

	{

		char* lectura = readline("--> ");

		if (string_is_empty(lectura)) {
			printf("No es una request valida, vuelva prontos \n");
			free(lectura);
			continue;
		}

		char** requestYParametros = string_n_split(lectura, 2, " ");

		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!esUnaRequestValida(requestYParametros[0],
				requestYParametros[1]) || requestEnInt == JOURNAL
				|| requestEnInt == ADD || requestEnInt == RUN || requestEnInt == METRICS) { //Si es invalida o es una request que no vale en el LFS

			printf("No es una request valida, vuelva prontos \n");

			liberarArrayDeStrings(requestYParametros);
			free(lectura);
			continue;
		}

		if (requestYParametros[1] == NULL) { //Para que no rompa en el string_duplicate de funcionesLFS.c
			requestYParametros[1] = (char *) malloc(sizeof(" "));
			strcpy(requestYParametros[1], " ");
		}

		mandarAEjecutarRequest(requestEnInt, requestYParametros[1]);

		liberarArrayDeStrings(requestYParametros);
		free(lectura);

	}

}
