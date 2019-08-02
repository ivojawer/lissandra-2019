#include "funcionesMemoria.h"
extern sem_t sem_refreshConfig;
extern int sleepJournal;
extern int sleepGossiping;
extern int retardoAccesoLFS;
extern int retardoAccesoMemoria;

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

		if(!strcmp(requestYParametros[0],"RETARDO_JOURNAL"))
		{
			if(esUnNumero(requestYParametros[1]))
			{
				sem_wait(&sem_refreshConfig);
				sleepJournal = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		}
		if(!strcmp(requestYParametros[0],"RETARDO_GOSSIPING"))
		{
			if(esUnNumero(requestYParametros[1]))
			{
				sem_wait(&sem_refreshConfig);
				sleepGossiping = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		}
		if(!strcmp(requestYParametros[0],"RETARDO_FS"))
		{
			if(esUnNumero(requestYParametros[1]))
			{
				sem_wait(&sem_refreshConfig);
				retardoAccesoLFS = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		}
		if(!strcmp(requestYParametros[0],"RETARDO_MEM"))
		{
			if(esUnNumero(requestYParametros[1]))
			{
				sem_wait(&sem_refreshConfig);
				retardoAccesoMemoria = atoi(requestYParametros[1]);
				sem_post(&sem_refreshConfig);
			}
		}

		int requestEnInt = queRequestEs(requestYParametros[0]);

		if ((!esUnaRequestValida(lectura) && strcmp(lectura, "STATUS") != 0)||
				requestEnInt == ADD || requestEnInt == RUN || requestEnInt == METRICS) { //Si es invalida o es una request que no vale en la memoria

			printf("No es una request valida, vuelva prontos \n");

			liberarArrayDeStrings(requestYParametros);
			free(lectura);

			continue;
		}


		requestConID* requestAEjecutar = malloc(sizeof(requestConID));
		if(strcmp(lectura, "STATUS") == 0){
			request* requestNuevo = malloc(sizeof(request));
			requestNuevo->parametros= string_duplicate(" ");
			requestNuevo->requestEnInt= STATUS;
			requestAEjecutar->laRequest = requestNuevo;
			requestAEjecutar->idKernel = 0;
		}else{
			requestAEjecutar->laRequest = crearStructRequest(lectura);
			requestAEjecutar->idKernel = 0;
		}
		ponerRequestEnColaDeEjecucion(requestAEjecutar);

		liberarArrayDeStrings(requestYParametros);
		free(lectura);
	}

}
