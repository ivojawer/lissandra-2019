#include "funcionesKernel.h"
extern int sleepEjecucion;
extern int intervaloDeRefreshMetadata;
extern int quantum;
extern sem_t sem_refreshConfig;
extern sem_t sem_actualizacionMetadatas;
extern t_list* listaTablas;

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

		if (!strcmp(lectura, "EXIT")) {
			free(lectura);
			return;
		}
		if (!strcmp(lectura, "METADATAS")) {
			sem_wait(&sem_actualizacionMetadatas);
			describirMetadatas(listaTablas);
			sem_post(&sem_actualizacionMetadatas);
			free(lectura);
			continue;
		}

		char** requestYParametros = string_n_split(lectura, 2, " ");

		int requestEnInt = queRequestEs(requestYParametros[0]);

		if (!strcmp(requestYParametros[0], "STATUS")) {
			status();

		} else if (!strcmp(requestYParametros[0], "JOURNAL") || !strcmp(requestYParametros[0], "FORCE JOURNAL") ) {
			journal();
		} else if (!strcmp(requestYParametros[0], "SLEEP")) {
			if (requestYParametros[1] != NULL
					&& esUnNumero(requestYParametros[1])) {
				sem_wait(&sem_refreshConfig);
				sleepEjecucion = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		} else if (!strcmp(requestYParametros[0], "METADATA_REFRESH")) {
			if (requestYParametros[1] != NULL
					&& esUnNumero(requestYParametros[1])) {
				sem_wait(&sem_refreshConfig);
				intervaloDeRefreshMetadata = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		} else if (!strcmp(requestYParametros[0], "QUANTUM")) {
			if (requestYParametros[1] != NULL
					&& esUnNumero(requestYParametros[1])) {
				sem_wait(&sem_refreshConfig);
				quantum = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		}

		else if (!esUnaRequestValida(lectura)) {

			printf("No es una request valida, vuelva prontos \n");

		}

		else if (requestEnInt == RUN && !existeArchivo(requestYParametros[1])) {
			printf("%s%s%s", "El archivo ", requestYParametros[1],
					" no pudo ser encontrado \n");
		}

		else if (requestEnInt == ADD) {
			add(requestYParametros[1]);
		}
		else if(requestEnInt == METRICS)
		{
			metrics(0);
		}

		else {

			request* requestParaHilo = crearStructRequest(lectura);

			pthread_t h_script;

			pthread_create(&h_script, NULL, (void *) crearScript,
					requestParaHilo); //El hilo se encarga de liberarlo

			pthread_detach(h_script);

		}

		liberarArrayDeStrings(requestYParametros);
		free(lectura);

	}

}
