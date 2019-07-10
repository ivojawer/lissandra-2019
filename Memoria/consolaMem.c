#include "funcionesMemoria.h"
extern t_list* colaDeRequests;
extern sem_t requestsDisponibles;
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

		if (!strcmp(lectura,"EXIT"))
		{
			return;
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
		requestConID* requestAEjecutar = malloc(sizeof(requestConID));
		requestAEjecutar->laRequest = crearStructRequest(lectura);
		requestAEjecutar->idKernel = 0;

		list_add(colaDeRequests,requestAEjecutar);
		sem_post(&requestsDisponibles);

		liberarArrayDeStrings(requestYParametros);
		free(lectura);
	}

}
