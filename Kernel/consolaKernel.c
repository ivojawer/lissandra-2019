#include "funcionesKernel.h"

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

		if (!esUnaRequestValida(requestYParametros[0], requestYParametros[1])) {

			printf("No es una request valida, vuelva prontos \n");

			free(requestYParametros[1]);
			free(requestYParametros[0]);
			free(requestYParametros);
			free(lectura);

			continue;
		}

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);

	}

}