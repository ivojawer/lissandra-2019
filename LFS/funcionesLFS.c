#include "funcionesLFS.h"

extern t_config* bitMapMetadata;
extern t_list* listaDeNombreDeTablas;
extern t_list* memTable;
extern t_log* logger;
extern int cantidadBloques;
extern char* puntoDeMontaje;
extern int socketLFSAMEM;
extern int valorMaximoValue;




//  "No chabon que paso aca antes era mas bonito este archivo" Mira cucha despues se hace mas bonito esto ok? No te preocupes como esta ahora pensa en el futuro







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
	string_append(&pathAbsoluto, puntoDeMontaje);
	string_append(&pathAbsoluto, "Tablas/");
	string_append(&pathAbsoluto, nombreTablaGuardado);

	if (access(pathAbsoluto, F_OK) == -1) {
		return 0;
	}
	return 1;

}

int tablaExisteEnMemTable(char* nombreDeTabla) {

	char* nombreTablaGuardado = string_duplicate(nombreDeTabla);
	string_to_upper(nombreTablaGuardado);

	bool coincideNombreTabla(t_tablaEnMemTable* unaTabla) {
		return string_equals_ignore_case(unaTabla->nombreTabla,
				nombreTablaGuardado);
	}

	if (list_any_satisfy(memTable, (void*) coincideNombreTabla)) {
		return 1;
	}
	return 0;

}

void crearTablaEnMemTable(char* nombreDeTabla) {

	t_tablaEnMemTable* nuevaTabla;

	nuevaTabla = malloc(sizeof(t_tablaEnMemTable));

	nuevaTabla->nombreTabla = malloc(sizeof(nombreDeTabla));

	nuevaTabla->nombreTabla = string_duplicate(nombreDeTabla);

	nuevaTabla->datosAInsertar = list_create();

	list_add(memTable, nuevaTabla);

	//creo que falta el to upper??

}

t_tablaEnMemTable* getTablaPorNombre(t_list* memoriaTemp, char* nombreDeTabla) {

	bool filtroNombreTabla(t_tablaEnMemTable* unaTabla) {
		return string_equals_ignore_case(unaTabla->nombreTabla, nombreDeTabla);
	}

	return list_find(memoriaTemp, (void*) filtroNombreTabla);
}

void Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);

	if (!tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
		return;
	}

	int particion = particionDeKey(key, tabla);

	free(parametrosEnVector[1]);
	free(parametrosEnVector[0]);
	free(parametrosEnVector);
	free(parametros);

}

int particionDeKey(int key, char* nombreTabla) {


	metadataTablaLFS laMetadata = describirTabla(nombreTabla);

	free(laMetadata.nombre);

	return key % laMetadata.particiones;


}

void insert(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* nomTabla = parametrosEnVector[0];
	int unaKey = atoi(parametrosEnVector[1]);
	char* unValue = get_value(parametros);
	int unTimestamp;

	if (strlen(unValue) > valorMaximoValue) {
		printf("El valor ingresado sobrepasa el tamaño maximo permitido.\n");
		return;
	}

	if (parametrosEnVector[3] == NULL) { //Esto se podria delegar
		unTimestamp = get_timestamp();
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
	nuevoRegistro->value = unValue;

	if (!tablaExisteEnMemTable(nomTabla)) {
		crearTablaEnMemTable(nomTabla);
	}

	t_tablaEnMemTable* tabla = getTablaPorNombre(memTable, nomTabla);
	list_add(tabla->datosAInsertar, nuevoRegistro);
}

//t_tablaEnMemTable* ultimaTabla(t_list* memTemp){
//	return list_get(memTemp,memTemp->elements_count - 1);
//}
//
//t_tablaEnMemTable* ultimoDato(t_list* datos){
//	return list_get(datos,datos->elements_count - 1);
//}

void testearBitMap(t_bitarray* bitMap) {
	for (int i = 0; i < cantidadBloques; i++) {
		int bit = bitarray_test_bit(bitMap, i);
		printf("bittt:%i\n", bit);
	}
}

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
	string_append(&pathAbsoluto, puntoDeMontaje);
	string_append(&pathAbsoluto, "Tablas/");
	string_append(&pathAbsoluto, tabla);

	int result = mkdir(pathAbsoluto, 0700);

	if (-1 == result) {
		printf("Error creating directory!\n");
		exit(1);
	}

	//crea metadata y la escribe
	char* archivoMetadata = string_new();
	string_append(&archivoMetadata, pathAbsoluto);
	string_append(&archivoMetadata, "/metadata");

	FILE* metadata = fopen(archivoMetadata, "a+");

	fprintf(metadata, "CONSISTENCY=%s \n", consistencia);
	fprintf(metadata, "PARTITIONS=%s \n", particiones);
	fprintf(metadata, "COMPACTION_TIME=%s \n", tiempoCompactacion);

	fclose(metadata);

	//crea los .bin
	int cantParticiones = atoi(particiones);

	for (int i = 0; i < cantParticiones; i++) {

		char numParticion[cantParticiones]; //no hay malloc, no va free
		sprintf(numParticion, "%i.bin", i);
		char* particion = string_new();
		string_append(&particion, pathAbsoluto);
		string_append(&particion, "/");
		string_append(&particion, numParticion);

		FILE* bin = fopen(particion, "a+");

		fprintf(bin, "SIZE=0 \n");

//		char numBloque[cantidadBloques];
//		int unBloque = encontrarBloqueLibre();
//		sprintf(numBloque,"%i",unBloque);
//		fprintf(bin,"BLOCKS=[%s]\n",numBloque);

		fclose(bin);

		free(particion);
	}

	liberarArrayDeStrings(parametrosEnVector);
	free(pathAbsoluto);
	free(archivoMetadata);
	//free(parametros);
	//seguro faltan frees aca

}

t_list* describe(char* parametro) { //Por que devuelve una lista esta funcion? Porque son las 12am y tengo que tomar atajos para llegar, despues se hace mas bonito -Tom

	//Por cierto asi como esta, esto genera tantos memory leaks como le plazca a la pc asi que esto es super malo


	if (esDescribeGlobal(parametro)) {

		return describirTodasLasTablas();
	}


	t_list* unaMetadataSolitaria = list_create();

	metadataTablaLFS laMetadataSolitaria = describirTabla(parametro);

	list_add(unaMetadataSolitaria,&laMetadataSolitaria);

	return unaMetadataSolitaria;

}

void drop(char* parametro) { //Perdon por esto

	DIR *dir;
	struct dirent *sd;
	char *root = string_new();
	string_append(&root, "../Tablas/");
	string_append(&root, parametro);

	t_list* listaDeArraysDeBloquesALiberar = list_create();

	dir = opendir(root);

	string_append(&root, "/");

	char *aux = string_new();
	while ((sd = readdir(dir)) != NULL) {

		if ((strcmp((sd->d_name), ".") != 0)
				&& (strcmp((sd->d_name), "..") != 0)) {
			string_append(&aux, root);
			string_append(&aux, sd->d_name);

			if (strcmp(aux, "Metadata")) {
				t_config* archivoDeTabla = config_create(aux);
				char** arrayBloquesALiberar = config_get_array_value(
						archivoDeTabla, "BLOCKS");
				list_add(&listaDeArraysDeBloquesALiberar, arrayBloquesALiberar);
				config_destroy(archivoDeTabla);
			}

			remove(aux);
			free(aux);
			aux = string_new();
		}
	}

	liberarBloques(listaDeArraysDeBloquesALiberar);

	rmdir(root);

	free(root);
	free(aux);
	closedir(dir);

}

void liberarBloques(t_list* listaDeArraysDeBloques) {
	t_list* listaBloquesALiberar = list_create();

	for (int i = 0; i < list_size(listaDeArraysDeBloques); i++) {

		char*** arrayDeBloques = list_get(listaDeArraysDeBloques, i);

		for (int i = 0; *arrayDeBloques[i] != NULL; i++) {

			int bloqueAAgregar = atoi(*arrayDeBloques[i]);

			list_add(listaBloquesALiberar, &bloqueAAgregar);

		}

		free(arrayDeBloques);
	}

	for (int i = 0; list_size(listaBloquesALiberar); i++) {
		int* bloqueALiberar = list_get(listaBloquesALiberar, i);

		bitarray_clean_bit(bitarray, *bloqueALiberar);
	}

	list_destroy(listaBloquesALiberar);

}

metadataTablaLFS describirTabla(char* nombreTabla) { //Precaucion! Esta funcion como esta ahora necesita Conocimiento Experto(tm) para ser usada, consultar a Tom antes de usar (es un tema de memory leaks)
	char* direccion = string_new();
	string_append(&direccion, "../Tablas/");
	string_append(&direccion, nombreTabla);
	string_append(&direccion, "/Metadata");

	t_config* metadataTabla = config_create(direccion);

	char* consistenciaEnString = config_get_string_value(metadataTabla,
			"CONSISTENCY");
	int particiones = config_get_int_value(metadataTabla, "PARTITIONS");

	int compactTime = config_get_int_value(metadataTabla, "COMPACTION_TIME");



	printf("Metadata de %s: ", nombreTabla);

	printf("\n\nConsistencia: %s", consistenciaEnString);

	printf("\nParticiones: %i",particiones);

	printf("\nCompaction time: %i", compactTime);

	int consistenciaEnInt = queConsistenciaEs(consistenciaEnString);


	metadataTablaLFS metadataADevolver;

	metadataADevolver.compactTime = compactTime;

	metadataADevolver.particiones = particiones;

	metadataADevolver.consistencia = consistenciaEnInt;

	metadataADevolver.nombre = nombreTabla;



	config_destroy(metadataTabla);

	free(consistenciaEnString);
	free(direccion);


	return metadataADevolver;
}

t_list* describirTodasLasTablas() { //Leer el comentario de decribirTabla(), se aplica lo mismo
	DIR *dir;
	struct dirent *sd;
	dir = opendir("../Tablas");

	t_list* metadatasADevolver = list_create();

	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0)
				&& (strcmp((sd->d_name), "..") != 0)) {
			list_add(metadatasADevolver,describirTabla(sd->d_name));
		}
	}
	closedir(dir);

	return metadatasADevolver;
}

//TODO: faltan todos los frees

//fseek(bin, 0, SEEK_END);
//int tamanioArchivo = ftell(bin);
//
//char* bytes[20]; //este 20 es sumamente cuestionable, preguntar
//sprintf(bytes, "%i", tamanioArchivo);
