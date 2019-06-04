#include "funcionesMemoria.h"

void consola() {

	while (1)

	{
		char* lectura = readline("--> ");

		if (lectura) {
			add_history(lectura);
		}

		if (string_is_empty(lectura)) {
			printf("No es una request valida, vuelva prontos \n");
			free(lectura);
			continue;
		}

		char** requestYParametros = string_n_split(lectura, 2, " ");

		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!esUnaRequestValida(
				lectura) || requestEnInt == ADD || requestEnInt == RUN || requestEnInt == METRICS) { //Si es invalida o es una request que no vale en la memoria

			printf("No es una request valida, vuelva prontos \n");

			liberarArrayDeStrings(requestYParametros);
			free(lectura);

			continue;
		}
		mandarAEjecutarRequest(crearStructRequest(lectura));

		liberarArrayDeStrings(requestYParametros);
		free(lectura);
	}

}
