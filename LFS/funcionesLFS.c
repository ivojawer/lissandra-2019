#include "funcionesLFS.h"

extern t_list* listaDeNombreDeTablas;
extern t_list* memTable;
extern t_log* logger;

extern int socketLFSAMEM;



void mandarAEjecutarRequest(request* requestAEjecutar) {


	char* parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) Select, parametros);

			pthread_detach(h_select);

			//enviarString("Me llego un select, ocupate macho", socketLFSAMEM);

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

int tablaYaExiste(char* nombreTabla) {

	char* nombreTablaGuardado = string_duplicate(nombreTabla);
	string_to_upper(nombreTablaGuardado);

	char* pathAbsoluto = string_new();

	// char* rutaPrincipal = (????); TODO: poner la primera parte (ruta principal) sino esta wea no funciona
	// string_append(&pathAbsoluto,rutaPrincipal);  y bueno aca se agregaria la primera parte mentienden

	// Aca va lo feo hardcodeado que NO TIENE QUE IR pero quiero ver si funciona chiks:
	string_append(&pathAbsoluto,"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/Debug/");
	string_append(&pathAbsoluto,"Tablas/");
	string_append(&pathAbsoluto,nombreTablaGuardado);

	if (access(pathAbsoluto, F_OK) == -1){
       return 0;
	}
	return 1; // devuelve 1 si existe la tabla, sino devuelve 0.

}

int tablaExisteEnMemTable(char* nombreDeTabla){

	char* nombreTablaGuardado = string_duplicate(nombreDeTabla);
	string_to_upper(nombreTablaGuardado);

	bool coincideNombreTabla(t_tablaEnMemTable* unaTabla){
		return string_equals_ignore_case(unaTabla->nombreTabla,nombreTablaGuardado);
	}

	if(list_any_satisfy(memTable,(void*) coincideNombreTabla)){
		return 1;
	}
	return 0; // devuelve 1 si existe la tabla, sino devuelve 0.

}


void crearTablaEnMemTable(char* nombreDeTabla) {

	t_tablaEnMemTable* nuevaTabla;

	nuevaTabla = malloc(sizeof(t_tablaEnMemTable));

	nuevaTabla->nombreTabla = malloc(sizeof(nombreDeTabla));

	nuevaTabla->nombreTabla = string_duplicate(nombreDeTabla);

	nuevaTabla->datosAInsertar = list_create();

	list_add(memTable, nuevaTabla);

}

t_tablaEnMemTable* getTablaPorNombre(t_list* memoriaTemp, char* nombreDeTabla){

	bool filtroNombreTabla(t_tablaEnMemTable* unaTabla){
		return string_equals_ignore_case(unaTabla->nombreTabla,nombreDeTabla);
	}

	return list_find(memoriaTemp,(void*)filtroNombreTabla);
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

	char* nomTabla = parametrosEnVector[0];
	int unaKey = atoi(parametrosEnVector[1]);
	char* unValue = parametrosEnVector[2]; //TODO: Sacarle las comillas
	int unTimestamp;

	if (parametrosEnVector[3] == NULL) {
		unTimestamp = 123; //TODO: poner aca el tiempo actual
	} else {
		unTimestamp = atoi(parametrosEnVector[3]);
	}

	if (!tablaYaExiste(nomTabla)) {
		log_error(logger, "%s%s%s", "La tabla ", nomTabla, " no existe.");
		return;
	}

	//TODO: Dice obtener la metadata asociada a dicha tabla. ?? dafuq - preguntar

	dato* nuevoDato;
	nuevoDato = malloc(sizeof(dato));
	nuevoDato->timestamp = unTimestamp;
	nuevoDato->key = unaKey;
	nuevoDato->value = malloc(sizeof(unValue));
	nuevoDato->value = unValue;

	if (!tablaExisteEnMemTable(nomTabla)){
		crearTablaEnMemTable(nomTabla);
	}

	t_tablaEnMemTable* tabla = getTablaPorNombre(memTable,nomTabla);
	list_add(tabla->datosAInsertar,nuevoDato);
}

//t_tablaEnMemTable* ultimaTabla(t_list* memTemp){
//	return list_get(memTemp,memTemp->elements_count - 1);
//}
//
//t_tablaEnMemTable* ultimoDato(t_list* datos){
//	return list_get(datos,datos->elements_count - 1);
//}

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

	if (-1 == result){
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

//TODO: faltan todos los frees
