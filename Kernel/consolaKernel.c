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

		if (!strcmp(requestYParametros[0], "EXIT")) {
			//Aca hay que matar a los hijos

			liberarArrayDeStrings(requestYParametros);
			free(lectura);
			return;
		}

		if (!strcmp(requestYParametros[0], "METRICS")) {

			metrics();
			liberarArrayDeStrings(requestYParametros);
			free(lectura);
		}

		if (!esUnaRequestValida(requestYParametros[0], requestYParametros[1])) {

			printf("No es una request valida, vuelva prontos \n");

			liberarArrayDeStrings(requestYParametros);
			free(lectura);

			continue;
		}

		if (requestEnInt == RUN && !existeArchivo(requestYParametros[1])) {
			printf("%s%s%s", "El archivo ", requestYParametros[1],
					" no pudo ser encontrado \n");

			liberarArrayDeStrings(requestYParametros);
			free(lectura);

			continue;
		}

		if (requestYParametros[1] == NULL) { //Para que no rompa
			requestYParametros[1] = (char *) malloc(sizeof(" "));
			strcpy(requestYParametros[1], " ");
		}

		parametros_hiloScript* parametrosParaHilo = malloc(
				sizeof(parametros_hiloScript));

		parametrosParaHilo->request = requestEnInt;
		parametrosParaHilo->parametros = string_duplicate(
				requestYParametros[1]);

		pthread_t h_script;

		pthread_create(&h_script, NULL, (void *) crearScript,
				parametrosParaHilo);

		pthread_detach(h_script);

		//Agregar lista de hilos?

		liberarArrayDeStrings(requestYParametros);
		free(lectura);

	}

}
