#include "funcionesLFS.h"


extern t_list* listaDeNombreDeTablas;
extern t_list* memTable;
extern t_log* logger;

extern int socketLFSAMEM;



void mandarAEjecutarRequest(int request, char* parametrosOriginal) {
	printf("me mandaron un request nro:%d\n", request);

	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	char* parametros = string_duplicate(parametrosOriginal); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
	switch (request) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) Select, parametros);

			pthread_detach(h_select);

			enviarString("Me llego un select, ocupate macho", socketLFSAMEM);

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

int tablaYaExiste(char* nombreTabla) {

	string_to_upper(nombreTabla); //El nombre tiene que estar en mayuscula


	return 0;
}

void crearTablaEnMemTable(char* nombreDeTabla) {
	t_tablaEnMemTable* nuevaTabla;

	nuevaTabla = malloc(sizeof(t_tablaEnMemTable));

	nuevaTabla->nombreTabla = malloc(sizeof(nombreDeTabla));

	nuevaTabla->nombreTabla = string_duplicate(nombreDeTabla);

	nuevaTabla->datosAInsertar = list_create();

	list_add(memTable, nuevaTabla);
}

void Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);

	if (!tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
		return;
	}

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

	if (!tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
		return;
	}

}

void create(char* parametros) {
	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = string_duplicate(parametrosEnVector[0]);
	char* consistencia = parametrosEnVector[1];
	int particiones = atoi(parametrosEnVector[2]);
	char* tiempoCompactacion = parametrosEnVector[3];


	//chequea si existe
	if (tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " ya existe.");
		return;
	}

	//crea directorio
	char* pathCompleto = NULL;
	int tablaLen = strlen( tabla );
	pathCompleto = malloc( 7 + tablaLen  + 1 ); // Add 1 for null terminator.
	strcpy( pathCompleto, "Tablas/" );
	strcat( pathCompleto, tabla );

	printf("Path entero:%s\n", pathCompleto);
	int result = mkdir(pathCompleto, 0700);

	if (-1 == result)
	{

		printf("Error creating directory!\n");
	    exit(1);
	}

	//crea metadata





	liberarArrayDeStrings(parametrosEnVector);
	free(pathCompleto);//?
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
