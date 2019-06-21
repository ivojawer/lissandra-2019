#include "funcionesKernel.h"

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

		if (!strcmp(requestYParametros[0],"probar")){
			enviarPeticionesDeGossip();
			continue;
		}

		if (!strcmp(requestYParametros[0], "EXIT")) {
			//TODO: Aca hay que matar a los hijos, si se puede

			liberarArrayDeStrings(requestYParametros);
			free(lectura);
			return;

		} else if (!strcmp(requestYParametros[0], "STATUS")) {
			status();

		} else if (!strcmp(requestYParametros[0], "METRICS")) {
			metrics(0);

		} else if (!strcmp(requestYParametros[0], "ADD")) {
			add(requestYParametros[1]);

		} else if (!strcmp(requestYParametros[0], "JOURNAL")) {
			journal();
		}

		else if (!esUnaRequestValida(lectura)) {

			printf("No es una request valida, vuelva prontos \n");

		}

		else if (requestEnInt == RUN && !existeArchivo(requestYParametros[1])) {
			printf("%s%s%s", "El archivo ", requestYParametros[1],
					" no pudo ser encontrado \n");
		}

		else {

			request* requestParaHilo = crearStructRequest(lectura);

			pthread_t h_script;

			pthread_create(&h_script, NULL, (void *) crearScript,
					requestParaHilo); //El hilo se encarga de liberarlo

			pthread_detach(h_script);

			//Agregar lista de hilos?

		}

		liberarArrayDeStrings(requestYParametros);
		free(lectura);

	}

}
