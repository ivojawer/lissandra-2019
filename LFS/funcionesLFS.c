#include "funcionesLFS.h"
#define MAXCHAR 1000
extern t_config* bitMapMetadata;
extern t_list* listaDeNombreDeTablas;
extern t_list* memTable;
extern t_log* logger;
extern int cantidadBloques;
extern char* puntoDeMontaje;
extern int socketLFSAMEM;
extern t_bitarray* bitMap;


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
	string_append(&pathAbsoluto,puntoDeMontaje);
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

	char* tabla = string_duplicate(nombreDeTabla);

	string_to_upper(tabla);

	nuevaTabla->nombreTabla = tabla;

	nuevaTabla->datosAInsertar = list_create();

	list_add(memTable, nuevaTabla);

}

t_tablaEnMemTable* getTablaPorNombre(t_list* memoriaTemp, char* nombreDeTabla){

	bool filtroNombreTabla(t_tablaEnMemTable* unaTabla){
		return string_equals_ignore_case(unaTabla->nombreTabla,nombreDeTabla);
	}

	return list_find(memoriaTemp,(void*)filtroNombreTabla);
}

int posicionLibreEnBitMap(){
	int i;
	for(i=0;i<cantidadBloques;i++){
		if(!bitarray_test_bit(bitMap,i)){
			log_info(logger,"Posicion %i en el bitmap libre.",i);
			return i;
		}
	}
	return -1;
}

void Select(char* parametros) {

//	char** parametrosEnVector = string_n_split(parametros, 2, " ");
//
//	char* tabla = parametrosEnVector[0];
//	int key = atoi(parametrosEnVector[1]);
//
//	//chequea si existe
//	if (!tablaYaExiste(tabla)) {
//		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
//		return;
//	}
//
//	//sacar path de tabla - TODO: quizas esto lo tengo que abstraer en otra funcion?? (despues refactoreo)
//	char* pathAbsoluto = string_new();
//	string_append(&pathAbsoluto,puntoDeMontaje);
//	string_append(&pathAbsoluto,"Tablas/");
//	string_append(&pathAbsoluto,tabla);
//
//	//obtener metadata (cant particiones)
//	char* archivoMetadata = string_new();
//	string_append(&archivoMetadata,pathAbsoluto);
//	string_append(&archivoMetadata,"/metadata");
//
//	t_config* metadata = config_create(archivoMetadata);
//
//	//calcular particion con key
//	int cantParticiones=config_get_int_value(metadata,"PARTITIONS");
//	int numeroParticion = key % cantParticiones; //funcion modulo
//
//	char* archivoParticion = string_new();
//	string_append(&archivoParticion,pathAbsoluto);
//	char numParticion[cantParticiones];
//	sprintf(numParticion, "%i.bin",numeroParticion);
//	string_append(&archivoParticion,numParticion);
//
//
//	t_config* particion = config_create(archivoParticion);
//	printf("hola \n");
//
//	//buscar en los bloques
//
//	char** bloques = config_get_array_value(particion,"BLOQUES");
//	printf("hola \n");
//
//	char* pathBloques = string_new();
//	string_append(&pathBloques,puntoDeMontaje);
//	string_append(&pathBloques,"/Bloques/");
//
//	// proximamente: REFACTOREO DONDE ABSTRAIGO BUSCAR EN BLOQUE - Solo En Cines
//
//	printf("hola \n");
//
//	for(int i=0;bloques[i]!=NULL;i++){
//
//		printf("hola \n");
//
//		char* bloque = string_new();
//		string_append(&bloque,pathBloques);
//		string_append(&bloque,bloques[i]);
//		string_append(&bloque,".bin");
//
//		FILE* bin = fopen(bloque,"a+");
//		char str[MAXCHAR];
//
//		while (fgets(str, MAXCHAR, bin) != NULL){
//
//			printf("hola \n");
//
//			char** unRegistro = string_split(str,";"); //SEPARO LOS TRES VALORES DEL REGISTRO
//
//			char* keyParseada = string_substring(unRegistro[1],1,string_length(unRegistro[1])-1);
//
//			printf("hola \n");
//
//			printf("naooos: %s \n",keyParseada);
//
//		}
//
//		fclose(bin);
//
//	}
//
//
//	free(parametrosEnVector[1]);
//	free(parametrosEnVector[0]);
//	free(parametrosEnVector);
//	free(parametros);

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

	registro* nuevoRegistro;
	nuevoRegistro = malloc(sizeof(registro));
	nuevoRegistro->timestamp = unTimestamp;
	nuevoRegistro->key = unaKey;
	nuevoRegistro->value = malloc(sizeof(unValue)); //por aca falta contemplar que el size del value sea valido
	nuevoRegistro->value = unValue;

	if (!tablaExisteEnMemTable(nomTabla)){
		crearTablaEnMemTable(nomTabla);
	}

	t_tablaEnMemTable* tabla = getTablaPorNombre(memTable,nomTabla);
	list_add(tabla->datosAInsertar,nuevoRegistro);
}

void create(char* parametros) {
	char** parametrosEnVector = string_n_split(parametros, 4, " ");
	char* tabla = string_duplicate(parametrosEnVector[0]);
	string_to_upper(tabla);
	char* consistencia = parametrosEnVector[1];
	char* particiones = parametrosEnVector[2];
	char* tiempoCompactacion = parametrosEnVector[3];

	//chequea si existe
	if (tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " ya existe.");
		return;
	}

	//crea directorio
	char* pathAbsoluto = string_new();
	string_append(&pathAbsoluto,puntoDeMontaje);
	string_append(&pathAbsoluto,"Tablas/");
	string_append(&pathAbsoluto,tabla);

	int result = mkdir(pathAbsoluto, 0700);

	if (-1 == result){
		printf("Error creating directory!\n");
		exit(1);
	}

	//crea metadata y la escribe
	char* archivoMetadata = string_new();
	string_append(&archivoMetadata,pathAbsoluto);
	string_append(&archivoMetadata,"/metadata");

	FILE* metadata = fopen(archivoMetadata,"a+");

	fprintf(metadata,"CONSISTENCY=%s \n",consistencia);
	fprintf(metadata,"PARTITIONS=%s \n",particiones);
	fprintf(metadata,"COMPACTION_TIME=%s \n",tiempoCompactacion);

	fclose(metadata);

	//crea los .bin
	int cantParticiones = atoi(particiones);

	for(int i=0;i<cantParticiones;i++){

		char numParticion[cantParticiones]; //no hay malloc, no va free
		sprintf(numParticion, "%i", i);
		char* particion = string_new();
		string_append(&particion,pathAbsoluto);
		string_append(&particion,"/");
		string_append(&particion,numParticion);
		string_append(&particion,".bin");

		FILE* bin = fopen(particion,"a+");

		fprintf(bin,"SIZE=0 \n");

		int posicionBloque = posicionLibreEnBitMap();
		if(posicionBloque == -1){
				log_error(logger,"No hay bloques disponibles");
				return;
		}
		bitarray_set_bit(bitMap,posicionBloque);

		char numBloque[cantidadBloques];
		sprintf(numBloque,"%i",posicionBloque);
		fprintf(bin,"BLOCKS=[%s]\n",numBloque);

		fclose(bin);

		free(particion);
	}

	liberarArrayDeStrings(parametrosEnVector);
	free(pathAbsoluto);
	free(archivoMetadata);
	//free(parametros);
	//seguro faltan frees aca

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

//--------------Basura que quiero guardar--------------------------------------------

//----------Basura del bitMap en el config------------------------

//	t_bitarray* bitMap = generarBitMap();
//	bitarray_set_bit(bitMap,0);
//	guardarBitMapEnConfig(bitMap);
//	bitarray_destroy(bitMap);
//	t_bitarray* bitMap2 = generarBitMap();
//	testearBitMap(bitMap2);

//----------------------------------------------------------------

//t_bitarray* generarBitMap(){      //esto es rancio pero lo que sigue es peor lo100to
//
//	char* bitArray = config_get_string_value(bitMapMetadata,"BITMAP");
//	t_bitarray* bitMap = bitarray_create(bitArray,string_length(bitArray));
//	return bitMap;
//}
//
//void guardarBitMapEnConfig(t_bitarray* bitMap){ //ESTA FUNCION ES LO MAS FEO QUE HICE EN MI VIDA, SI QUIEREN POR DISCORD EXPLICO EL PORQUE
//
//	//genero UN STRING CON LA ESTRUCTURA DEL ARRAY A PARTIR DE UN BITMAP KHE (horrible ya se)
//	char* arrayEnString = string_new();
//	string_append(&arrayEnString,"[");
//	for(int i=0;i<cantidadBloques;i++){
//		int bit = bitarray_test_bit(bitMap,i);
//		string_append(&arrayEnString,string_itoa(bit));
//		string_append(&arrayEnString,",");
//	}
//	int largo =string_length(arrayEnString);
//	char* arrayEnStringSinComa = string_substring_until(arrayEnString,largo-1);
//	string_append(&arrayEnStringSinComa,"]");
//
//	//guardo esta criatura del diablo en el CONFIG
//	config_set_value(bitMapMetadata,"BITMAP",arrayEnStringSinComa);
//}

//----------------------------------------------------------------

//----------Basura para probar algunas funciones---------

//t_tablaEnMemTable* ultimaTabla(t_list* memTemp){
//	return list_get(memTemp,memTemp->elements_count - 1);
//}
//
//t_tablaEnMemTable* ultimoDato(t_list* datos){
//	return list_get(datos,datos->elements_count - 1);
//}
void testearBitMap(t_bitarray* bitMap){
	for(int i=0;i<cantidadBloques;i++){
		int bit = bitarray_test_bit(bitMap,i);
		printf("bittt:%i\n",bit);
	}
}

//fseek(bin, 0, SEEK_END);
//int tamanioArchivo = ftell(bin);
//
//char* bytes[20]; //este 20 es sumamente cuestionable, preguntar
//sprintf(bytes, "%i", tamanioArchivo);
