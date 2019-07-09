#include "compactacion.h"

extern char* puntoDeMontaje;

void mostrarRegistro(t_registro* reg){
	printf("		key:%d;ts:%d;value:%s\n",reg->key,reg->timestamp,reg->value);
}

void mostrarListaDeRegistros(t_list* listReg){
	printf("	PARTICION:\n");
	list_iterate(listReg,(void*)mostrarRegistro);
}

void mostrarListaDeListasDeRegistros(t_list* lista){
	printf("LISTA:\n");
	list_iterate(lista,(void*)mostrarListaDeRegistros);
}

void compactar(char* tabla){

	int cantidadTemporales = renombrarATmpc(tabla);

	if(cantidadTemporales < 1){
		printf("no hay tmpc \n ");

		return;
	}

	t_list* tablaParticiones = list_create();
	t_list* tablaTemporales = list_create();

	int particiones = obtener_particiones_metadata(tabla);

	for(int i = 0; i < particiones; i++){
		char** bloquesParticion = transformarParticionABloques(tabla,i);
		list_add(tablaParticiones, traerRegistrosBloques(bloquesParticion));
	}

	for(int i = 0; i < cantidadTemporales; i++){
		char** bloquesTemporal = transformarTemporalABloques(tabla,i);
		list_add(tablaTemporales, traerRegistrosBloques(bloquesTemporal));
	}


	mostrarListaDeListasDeRegistros(tablaParticiones);
	mostrarListaDeListasDeRegistros(tablaTemporales);


	//lista particiones

	//lista tmpc


}

char** transformarParticionABloques(char* tabla,int nro_particion){

	int cantBloques = 0;

	char* particion = string_duplicate(puntoDeMontaje);
	string_append(&particion, "Tablas/");
	string_append(&particion, tabla);
	string_append(&particion, "/part");
	string_append(&particion,string_itoa(nro_particion));
	string_append(&particion,".bin");

	t_config* metadataParticion = config_create(particion);
	char** bloques = config_get_array_value(metadataParticion,"BLOCKS");

	void contarBloques(char* unBloque){
		cantBloques += 1;
	}

	string_iterate_lines(bloques,(void*) contarBloques);

	char** pathsBloquesParticion = (char**)malloc((cantBloques + 1) * (sizeof(char*)));
	int i = 0;
	void accionPorBloque(char* bloq){

		char* bloque = string_duplicate(puntoDeMontaje);
		string_append(&bloque,"Bloques/bloque");
		string_append(&bloque, bloq);
		string_append(&bloque, ".bin");
		pathsBloquesParticion[i] = string_duplicate(bloque);
		//printf("path de bloque: %s\n",pathsBloquesParticion[i]);
		i++;
	}

	string_iterate_lines(bloques,(void*)accionPorBloque);

	pathsBloquesParticion[cantBloques] = NULL;
	config_destroy(metadataParticion);
	return pathsBloquesParticion;
}

char** transformarTemporalABloques(char* tabla,int nro_temporal){

	int cantBloques = 0;


	char* temporal = string_duplicate(puntoDeMontaje);
	string_append(&temporal, "Tablas/");
	string_append(&temporal, tabla);
	string_append(&temporal, "/");
	string_append(&temporal, tabla);
	string_append(&temporal,string_itoa(nro_temporal));
	string_append(&temporal,".tmpc");

	//printf("path:%s\n",temporal);
	t_config* metadataTemporal = config_create(temporal);

	char** bloques = config_get_array_value(metadataTemporal,"BLOCKS");
	void contarBloques(char* unBloque){
		cantBloques += 1;
	}

	string_iterate_lines(bloques,(void*)contarBloques);

	char** pathsBloquesTemporal = (char**)malloc((cantBloques + 1) * (sizeof(char*)));

	printf("primer bloque:%s\n",pathsBloquesTemporal[0]);
	int i = 0;
	void accionPorBloque(char* bloq){

		char* bloque = string_duplicate(puntoDeMontaje);
		string_append(&bloque,"Bloques/bloque");
		string_append(&bloque, bloq);
		string_append(&bloque, ".bin");

		pathsBloquesTemporal[i] = string_duplicate(bloque);
		printf("path de bloque: %s",pathsBloquesTemporal[i]);
		i++;
	}
	string_iterate_lines(bloques,(void*)accionPorBloque);
	pathsBloquesTemporal[cantBloques] = NULL;
	config_destroy(metadataTemporal);

	return pathsBloquesTemporal;
}

int renombrarATmpc(char* tabla){

	char* path = string_duplicate(puntoDeMontaje);
	string_append(&path, "Tablas/");
	string_append(&path, tabla);
	int cantTmp = contar_archivos_con_extension(path,"tmp");
	string_append(&path,"/");
	string_append(&path, tabla);

	for(int i = 0; i < cantTmp ; i++){
		char* tmpFile = string_duplicate(path);
		string_append(&tmpFile,string_itoa(i));
		string_append(&tmpFile, ".tmp");

		char* tmpcFile = string_duplicate(tmpFile);
		string_append(&tmpcFile, "c");

		rename(tmpFile,tmpcFile);
	}

	return cantTmp;
}


void string_append_char(char* string, char c){
	char str[2];
	str[0] =c;
	str[1]='\0';
	string_append(&string,str);
}

t_registro* stringRegistroAStruct(char* registro){    //llega sin el \n al final :)

	char** stringRegistroSeparado = string_n_split(registro,3,";");
	t_registro* reg = malloc(sizeof(uint16_t)+sizeof(unsigned long)+sizeof(char)*strlen(stringRegistroSeparado[2]));

	reg->timestamp=atoi(stringRegistroSeparado[0]);
	reg ->key=atoi( stringRegistroSeparado[1]);
	reg->value=string_duplicate(stringRegistroSeparado[2]);

	return reg;
}

char* structRegistroAString(t_registro* registro){

	char* reg = string_new();

	char* unaKey = string_itoa(registro->key); //no se si funciona con uint16
	char* unTimestamp = string_itoa(registro->timestamp);
	char* unValue = string_duplicate(registro->value); //es necesario? ah

	string_append(&reg, unTimestamp);
	string_append(&reg, ";");
	string_append(&reg, unaKey);
	string_append(&reg, ";");
	string_append(&reg, unValue);
	string_append(&reg, "\n");

	return reg;
}

t_list* traerRegistrosBloques(char** bloques){

	t_list* registros = list_create();
	char* registroActual = string_new();

	void leerBloque(char* nombreBloque){
		printf("bloque actual:%s\n",nombreBloque);
		FILE* bloqueActual = fopen(nombreBloque,"r");
		char nuevoCaracter = fgetc(bloqueActual);
		while(nuevoCaracter > EOF){
			printf("caracter leido:%c\n",nuevoCaracter);
			if(nuevoCaracter == '\n'){
				printf("termino registro\n");
				list_add(registros, stringRegistroAStruct(registroActual));
				free(registroActual);
				registroActual = string_new();
			}else{
				//printf("agrego caracter a reg:%c\n",nuevoCaracter);
				string_append_char(registroActual,nuevoCaracter);
				printf("registro con caracter agregado:%s\n", registroActual);
			}
			nuevoCaracter = fgetc(bloqueActual);
		}
		fclose(bloqueActual);

	}

	string_iterate_lines(bloques, (void*) leerBloque);
	list_add(registros, stringRegistroAStruct(registroActual));


	//free(registroActual);
	printf("termine de leer bloques de la particion\n");
	return registros;
}
