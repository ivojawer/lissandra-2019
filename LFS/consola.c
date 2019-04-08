#include "funcionesLFS.h"

void consola() {

	while (1)

	{

		char* lectura = readline("--> "); //TODO: Rompe si no se ingresa nada

		char** requestYParametros = string_n_split(lectura, 2, " ");

		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!esUnaRequestValida(requestYParametros[0],
				requestYParametros[1]) || requestEnInt == JOURNAL
				|| requestEnInt == ADD || requestEnInt == RUN) { //Si es invalida o es una request que no vale en el LFS

			printf("No es una request valida, vuelva prontos \n");

			free(requestYParametros[1]);
			free(requestYParametros[0]);
			free(requestYParametros);
			free(lectura);

			continue;
		}
		mandarAEjecutarRequest(requestEnInt, requestYParametros[1]);

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);

	}

}
