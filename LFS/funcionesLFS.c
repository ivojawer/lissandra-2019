#include "funcionesLFS.h"

extern t_list* listaDeNombreDeTablas;
extern t_log* logger;

void mandarAEjecutarRequest(int request, char* parametrosOriginal) {

	char* parametros = string_duplicate(parametrosOriginal); //Esto es para que se pueda hacer un free() en consola.c sin que rompa

	switch (request) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) select, parametros);

			pthread_detach(h_select);

			return;
		}

	case INSERT:
		;
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) insert, parametros);

			pthread_detach(h_insert);

			return;
		}

	case CREATE:
		;

		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) create, parametros);

			pthread_detach(h_create);

			return;
		}

	case DESCRIBE:
		;
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) describe, parametros);

			pthread_detach(h_describe);

			return;
		}

	case DROP:
		;
		{

			pthread_t h_drop;

			pthread_create(&h_drop, NULL, (void *) drop, parametros);

			pthread_detach(h_drop);

			return;

		}

	}
}

int tablaYaExiste(char* nombreTabla)
{
	string_to_upper(nombreTabla); //El nombre tiene que estar en mayuscula


	for (int i = 0; i < list_size(listaDeNombreDeTablas); i++ )
	{
		if (!strcmp(nombreTabla, list_get(listaDeNombreDeTablas,i)))
		{
			return 1;
		}
	}

	return 0;
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

	int timestamp = atoi(parametrosEnVector[3]);

}

void create(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = string_duplicate(parametrosEnVector[0]);
	char* consistencia = parametrosEnVector[1];
	int particiones = atoi(parametrosEnVector[2]);
	char* tiempoCompactacion = parametrosEnVector[3];

	if(tablaYaExiste(tabla))
	{
		log_error(logger,"%s%s%s","La tabla ", tabla, " ya existe.");
		return;
	}

	list_add(listaDeNombreDeTablas, tabla);




	free(parametrosEnVector[3]);
	free(parametrosEnVector[2]);
	free(parametrosEnVector[1]);
	free(parametrosEnVector[0]);
	free(parametrosEnVector);
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
