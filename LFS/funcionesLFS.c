#include "funcionesLFS.h"
#define MAXCHAR 1000
extern t_config* bitMapMetadata;
extern t_list* listaDeNombreDeTablas;
extern t_list* memTable;
extern t_log* logger;
extern int cantidadBloques;
extern char* puntoDeMontaje;
extern int socketLFSAMEM;
extern int valorMaximoValue;
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
		string_append(&pathAbsoluto, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
		string_append(&pathAbsoluto, nombreTabla);

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

	char* tabla = string_duplicate(nombreDeTabla);

	string_to_upper(tabla);

	nuevaTabla->nombreTabla = tabla;

	nuevaTabla->datosAInsertar = list_create();

	list_add(memTable, nuevaTabla);

}

t_tablaEnMemTable* getTablaPorNombre(t_list* memoriaTemp, char* nombreDeTabla) {

	bool filtroNombreTabla(t_tablaEnMemTable* unaTabla) {
		return string_equals_ignore_case(unaTabla->nombreTabla, nombreDeTabla);
	}

	return list_find(memoriaTemp, (void*) filtroNombreTabla);
}

int posicionLibreEnBitMap() {
	int i;
	for (i = 0; i < cantidadBloques; i++) {
		if (!bitarray_test_bit(bitMap, i)) {
			log_info(logger, "Posicion %i en el bitmap libre.", i);
			return i;
		}
	}
	return -1;
}

registro* Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");

	registro* registroNulo = NULL;

	char* tabla = string_duplicate(parametrosEnVector[0]);

	int keyDatoBuscado = atoi(parametrosEnVector[1]);

	liberarArrayDeStrings(parametrosEnVector);

	if (!tablaYaExiste(tabla)) {
		log_error(logger, "%s%s%s", "La tabla ", tabla, " no existe.");
		return registroNulo;
	}

	char *root = string_new();
	string_append(&root, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
	string_append(&root, tabla);

	string_append(&root, "/");

	int particion = particionDeKey(keyDatoBuscado, tabla);

	string_append(&root, string_itoa(particion));

	string_append(&root, ".bin");

	char* stringDatoMasAltoDeLaParticion = string_new();
	registro* datoMasAltoDeLaMemTable;

	int timestampMasAlto = 0;

	int i = 0;
	while (1) {

		char* lineaLeida = leerLinea(root, i);

		if (!strcmp(lineaLeida, "fin")) {
			free(lineaLeida);
			break;
		}

		char** elementosEnVector = string_n_split(lineaLeida, 3, ";");

		int timestamp = atoi(elementosEnVector[0]);
		int key = atoi(elementosEnVector[1]);

		liberarArrayDeStrings(elementosEnVector);

		if (key == keyDatoBuscado && timestamp > timestampMasAlto) {
			free(stringDatoMasAltoDeLaParticion);
			stringDatoMasAltoDeLaParticion = string_duplicate(lineaLeida);
			timestampMasAlto = timestamp;
		}

		free(lineaLeida);
		i++;

	}

	t_tablaEnMemTable* laTabla = NULL;

	for (int d = 0; d < list_size(memTable); d++) {
		t_tablaEnMemTable* unaTabla = list_get(memTable, d);

		if (!strcmp(unaTabla->nombreTabla, tabla))

		{
			laTabla = unaTabla;
		}
	}

	if (laTabla == NULL) {
		if (strcmp(stringDatoMasAltoDeLaParticion, "\0")) {

			printf("SELECT de la key %i de la tabla %s: %s", keyDatoBuscado,
					tabla, stringDatoMasAltoDeLaParticion);

			char** parametrosEnVector = string_n_split(
					stringDatoMasAltoDeLaParticion, 3, ";");

			int unaKey = atoi(parametrosEnVector[1]);
			char* unValue = get_value(parametros);
			int unTimestamp = atoi(parametrosEnVector[0]);

			registro* nuevoRegistro;
			nuevoRegistro = malloc(sizeof(registro));
			nuevoRegistro->timestamp = unTimestamp;
			nuevoRegistro->key = unaKey;
			nuevoRegistro->value = unValue;

			return nuevoRegistro;
		} else {
			printf("No se encontro la key %i de la tabla %s", keyDatoBuscado,
					tabla);
			return registroNulo;
		}
	}

	for (int d = 0; d < list_size(laTabla->datosAInsertar); d++) {
		registro* unDatazo = list_get(laTabla->datosAInsertar, d);

		if ((unDatazo->timestamp > timestampMasAlto)
				&& (unDatazo->key == keyDatoBuscado)) {
			datoMasAltoDeLaMemTable = unDatazo;
		}
	}

	if (datoMasAltoDeLaMemTable == NULL
			&& strcmp(stringDatoMasAltoDeLaParticion, "\0")) {

		printf("No se encontro la key %i de la tabla %s", keyDatoBuscado,
				tabla);
		return registroNulo;
	}

	if (datoMasAltoDeLaMemTable == NULL) {
		printf("SELECT de la key %i de la tabla %s: %s", keyDatoBuscado, tabla,
				stringDatoMasAltoDeLaParticion);

		char** parametrosEnVector = string_n_split(
				stringDatoMasAltoDeLaParticion, 3, ";");

		int unaKey = atoi(parametrosEnVector[1]);
		char* unValue = get_value(parametros);
		int unTimestamp = atoi(parametrosEnVector[0]);

		registro* nuevoRegistro;
		nuevoRegistro = malloc(sizeof(registro));
		nuevoRegistro->timestamp = unTimestamp;
		nuevoRegistro->key = unaKey;
		nuevoRegistro->value = unValue;

		return nuevoRegistro;
	}

	printf("SELECT de la key %i de la tabla %s: %s",
			datoMasAltoDeLaMemTable->key, tabla,
			datoMasAltoDeLaMemTable->value);

	return datoMasAltoDeLaMemTable;
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
	string_append(&pathAbsoluto, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
	string_append(&pathAbsoluto, tabla);

	int result = mkdir(pathAbsoluto, ACCESSPERMS);

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
		sprintf(numParticion, "%i", i);
		char* particion = string_new();
		string_append(&particion, pathAbsoluto);
		string_append(&particion, "/");
		string_append(&particion, numParticion);
		string_append(&particion, ".bin");

		FILE* bin = fopen(particion, "a+");

		fprintf(bin, "SIZE=0 \n");

		int posicionBloque = posicionLibreEnBitMap();
		if (posicionBloque == -1) {
			log_error(logger, "No hay bloques disponibles");
			return;
		}
		bitarray_set_bit(bitMap, posicionBloque);

		char numBloque[cantidadBloques];
		sprintf(numBloque, "%i", posicionBloque);
		fprintf(bin, "BLOCKS=[%s]\n", numBloque);

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

	if (!strcmp(parametro, " ")) {

		return describirTodasLasTablas();
	}

	t_list* unaMetadataSolitaria = list_create();

	metadataTablaLFS laMetadataSolitaria = describirTabla(parametro);

	list_add(unaMetadataSolitaria, &laMetadataSolitaria);

	return unaMetadataSolitaria;

}

void drop(char* parametro) { //Perdon por esto

	DIR *dir;
	struct dirent *sd;
	char *root = string_new();
	string_append(&root, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
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
				list_add(listaDeArraysDeBloquesALiberar, &arrayBloquesALiberar);
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

		bitarray_clean_bit(bitMap, *bloqueALiberar);
	}

	list_destroy(listaBloquesALiberar);

}

metadataTablaLFS describirTabla(char* nombreTabla) { //Precaucion! Esta funcion como esta ahora necesita Conocimiento Experto(tm) para ser usada, consultar a Tom antes de usar (es un tema de memory leaks)
	char* direccion = string_new();
	string_append(&direccion, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
	string_append(&direccion, nombreTabla);
	string_append(&direccion, "/metadata");

	t_config* metadataTabla = config_create(direccion);

	char* consistenciaEnString = config_get_string_value(metadataTabla,
			"CONSISTENCY");
	int particiones = config_get_int_value(metadataTabla, "PARTITIONS");

	int compactTime = config_get_int_value(metadataTabla, "COMPACTION_TIME");

	printf("Metadata de %s: ", nombreTabla);

	printf("\n\nConsistencia: %s", consistenciaEnString);

	printf("\nParticiones: %i", particiones);

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

			metadataTablaLFS unaMetadata = describirTabla(sd->d_name);

			list_add(metadatasADevolver, &unaMetadata);
		}
	}
	closedir(dir);

	return metadatasADevolver;
}

void dumpTemporal(t_tablaEnMemTable* tablaADumpear) { //La funcion de la ineficiencia

	for (int i = 0; i < list_size(tablaADumpear->datosAInsertar); i++) {
		registro* registroAGuardar = list_get(tablaADumpear->datosAInsertar, i);

		int particionDelRegistro = particionDeKey(registroAGuardar->key,
				tablaADumpear->nombreTabla);

		char *root = string_new();
		string_append(&root, "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/LFS/raiz/tables/");
			string_append(&root, tablaADumpear->nombreTabla);

		mkdir(root, ACCESSPERMS);

		char* direccionArchivo = string_duplicate(root);

		free(root);

		string_append(&direccionArchivo, "/");
		string_append(&direccionArchivo, string_itoa(particionDelRegistro));
		string_append(&direccionArchivo, ".bin");

		FILE* archivoOriginal = fopen(direccionArchivo, "r");

		char* registroEnString = string_new();

		string_append(&registroEnString,
				string_itoa(registroAGuardar->timestamp));
		string_append(&registroEnString, ";");
		string_append(&registroEnString, string_itoa(registroAGuardar->key));
		string_append(&registroEnString, ";");
		string_append(&registroEnString, registroAGuardar->value);
		string_append(&registroEnString, "\n");

		if (archivoOriginal == NULL) {
			archivoOriginal = fopen(direccionArchivo, "w");
			fputs(registroEnString, archivoOriginal);
			fclose(archivoOriginal);
			return;
		}

		fclose(archivoOriginal);

		char* nuevaDireccionArchivo = string_duplicate(direccionArchivo);
		string_append(&nuevaDireccionArchivo, "tmp");

		FILE* archivoNuevo = fopen(nuevaDireccionArchivo, "w");

		while (1) {

			char* lineaLeida = leerLinea(direccionArchivo, i);

			if (!strcmp(lineaLeida, "fin")) {
				free(lineaLeida);
				break;
			}

			char** elementosEnVector = string_n_split(lineaLeida, 3, ";");

			int timestamp = atoi(elementosEnVector[0]);
			int key = atoi(elementosEnVector[1]);

			liberarArrayDeStrings(elementosEnVector);

			if (!(key == registroAGuardar->key
					&& timestamp > registroAGuardar->timestamp)) {
				string_append(&lineaLeida, "\n");
				fputs(lineaLeida, archivoNuevo);
			}

			else {
				fputs(registroEnString, archivoOriginal);
			}

			free(lineaLeida);
			i++;

		}

		fclose(archivoNuevo);

		remove(direccionArchivo);
		rename(nuevaDireccionArchivo, direccionArchivo);

		free(direccionArchivo);
		free(nuevaDireccionArchivo);

	}

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
void testearBitMap(t_bitarray* bitMap) {
	for (int i = 0; i < cantidadBloques; i++) {
		int bit = bitarray_test_bit(bitMap, i);
		printf("bittt:%i\n", bit);
	}
}

//fseek(bin, 0, SEEK_END);
//int tamanioArchivo = ftell(bin);
//
//char* bytes[20]; //este 20 es sumamente cuestionable, preguntar
//sprintf(bytes, "%i", tamanioArchivo);
