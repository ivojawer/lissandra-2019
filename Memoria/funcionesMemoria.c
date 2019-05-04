#include "funcionesMemoria.h"


extern t_log* logger;




void mandarAEjecutarRequest(request* requestAEjecutar) {

	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	char* parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa

	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) Select, parametros);

			pthread_detach(h_select);



			break;
		}

	case INSERT:
		;
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) insert, parametros);

			pthread_detach(h_insert);

			break;
		}

	case CREATE:
		;

		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) create, parametros);

			pthread_detach(h_create);

			break;
		}

	case DESCRIBE:
		;
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) describe, parametros);

			pthread_detach(h_describe);

			break;
		}

	case DROP:
		;
		{

			pthread_t h_drop;

			pthread_create(&h_drop, NULL, (void *) drop, parametros);

			pthread_detach(h_drop);

			break;

		}

	}

	liberarRequest(requestAEjecutar);
}


void Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);



	free(parametrosEnVector[1]);
	free(parametrosEnVector[0]);
	free(parametrosEnVector);
	free(parametros);

}

void insert(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);



	char* value = parametrosEnVector[2]; //TODO: Sacarle las comillas

	int timestamp;

	if (parametrosEnVector[3] == NULL) {
		timestamp = 123; //TODO: poner aca el tiempo actual
	}

	else {
		timestamp = atoi(parametrosEnVector[3]);
	}

//	if (!tablaYaExiste(tabla)) {
//		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
//		return;
//	}

}

void create(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = string_duplicate(parametrosEnVector[0]);
	char* consistencia = parametrosEnVector[1];
	int particiones = atoi(parametrosEnVector[2]);
	char* tiempoCompactacion = parametrosEnVector[3];

//	if (tablaYaExiste(tabla)) {
//		log_error(logger, "%s%s%s", "La tabla ", tabla, " ya existe.");
//		return;
//	}


	liberarArrayDeStrings(parametrosEnVector);
	free(parametros);

}

void describe(char* parametro) {

	if (strcmp(parametro, " ")) //Si hay un parametro
			{

		return;
	}

}

void drop(char* parametro) {

}
