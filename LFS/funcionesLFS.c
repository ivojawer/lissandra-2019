#include "funcionesLFS.h"

void mandarAEjecutarRequest(int request, char* parametrosOriginal) {

	char * parametros = string_duplicate(parametrosOriginal); //Esto es para que se pueda hacer un free() en consola.c sin que rompa

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

void Select(char* parametros) {

}

void insert(char* parametros) {

}

void create(char* parametros) {


}

void describe(char* parametro) {

}

void drop(char* parametro) {

}
