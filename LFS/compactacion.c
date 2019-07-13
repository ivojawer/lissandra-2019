#include "compactacion.h"

extern char* puntoDeMontaje;
extern int tamanioBloques;
extern int cantidadBloques;



void compactar(char* tabla){

	int cantidadTemporales = renombrarATmpc(tabla);
	if(cantidadTemporales < 1){	return;	}

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

	void evaluarRegistro(t_registro* reg){

		int particionABuscar = nr_particion_key(reg->key, particiones);

		t_list* listaRegistros = list_duplicate(list_get(tablaParticiones,particionABuscar));

		int encontroLaKey = 0;

		for(int i=0;i < list_size(listaRegistros);i++){
			t_registro* registro = list_get(listaRegistros,i);
			if(registro->key == reg->key){
				encontroLaKey = 1;
				if(reg->timestamp > registro->timestamp){
					list_replace(list_get(tablaParticiones,particionABuscar),i, reg);  //falta el destroy pero no se hacer la funcion destroyer
				}
			}
		}
		if(!encontroLaKey){
			list_add(list_get(tablaParticiones,particionABuscar),reg);
		}
	}

	void recorrerListaDeRegistros(t_list* listReg){
		list_iterate(listReg,(void*)evaluarRegistro);
	}
	list_iterate(tablaTemporales,(void*)recorrerListaDeRegistros);

	mostrarListaDeListasDeRegistros(tablaParticiones);

	// aca se deberia: Bloquear la tabla para cualquier operación sobre la misma

	//liberar bloques de todos:
	void vaciarBloque(char* bloque){
		t_bloque* bloq = crear_bloque_buscar(bloque);
		desmarcar_bloque_bitmap(bloq);
		bloque_destroy(bloq);
	}
	for(int i = 0; i < particiones; i++){
		char** bloquesParticion = getBloquesParticion(tabla,i);
		string_iterate_lines(bloquesParticion, (void*)vaciarBloque);
	}
	for(int i = 0; i < cantidadTemporales; i++){
		char** bloquesTemporal = getBloquesTemporal(tabla,i);
		string_iterate_lines(bloquesTemporal, (void*)vaciarBloque);
	}

	t_list* particionesEnChar = list_create();
	void transformarAStrings(t_list* lista){
		list_add(particionesEnChar,list_map(lista, (void*)structRegistroAString));
	}
	list_iterate(tablaParticiones,(void*)transformarAStrings);

	printf("lista transformada:\n");
	mostrarListaDeListasDeStrings(particionesEnChar);

	int bytesAEscribir = 0;
	t_list* bytes_por_particion = list_create();

	void contarLargoRegistros(char* registro){
		printf("registro:%slongitud:%d\n",registro,string_length(registro));
		bytesAEscribir += string_length(registro);
	}
	void iterarLaLista(t_list* lista){
		list_iterate(lista, (void*)contarLargoRegistros);
		int* punteroByte = malloc(sizeof(int));
		*punteroByte= bytesAEscribir;
		list_add(bytes_por_particion,punteroByte);
		printf("contador bytes:%d\n",bytesAEscribir);
		bytesAEscribir = 0;
	}
	list_iterate(particionesEnChar,(void*)iterarLaLista);

	int cantidadBloquesNecesarios = 0;

	void bloquesNecesariosPorParticion(int* bytesDeParticion){
	cantidadBloquesNecesarios += *bytesDeParticion / tamanioBloques;
	//printf("bdp:%d-tb:%d-bdp/tb:%d-bdp%tb:%d\n",*bytesDeParticion,tamanioBloques,*bytesDeParticion/tamanioBloques,*bytesDeParticion % tamanioBloques);
		if(*bytesDeParticion % tamanioBloques > 0){
			cantidadBloquesNecesarios += 1;
		}
	}
	list_iterate(bytes_por_particion,(void*)bloquesNecesariosPorParticion);

	if(controlar_bloques_disponibles(cantidadBloquesNecesarios) == 0){
		printf("Se excede la cantidad de bloques necesaria para compactar \n");
		//desbloquear en caso de que no se haga la compactacion
		return;
	}


	escribirEnBloquesTabla(particionesEnChar,tabla);

	list_iterate(bytes_por_particion,(void*)mostrarBytes);
	printf("cantidad de bloques necesarios: %i\n",cantidadBloquesNecesarios);

	//list_destroy_and_destroy_elements(bytes_por_particion,(void*)funcionDestroyerInts);
	//list_destroy_and_destroy_elements(tablaParticiones,(void*)funcionDestroyerLista); // hay que borrar una lista de listas
	//list_destroy_and_destroy_elements(tablaTemporales,(void*)funcionDestroyerLista); // hay que borrar una lista de listas

	//Desbloquear la tabla y dejar un registro de cuánto tiempo estuvo bloqueada la tabla para realizar esta operatoria

}

void escribirEnBloquesTabla(t_list* tablaParticiones,char* nombreTabla){
	for(int i = 0; i < list_size(tablaParticiones); i++){
		escribirEnBloquesParticion(list_get(tablaParticiones,i),i,nombreTabla);
	}
}

void escribirEnBloquesParticion(t_list* registrosDeParticion, int numeroParticion, char* tabla){

	int cantidadBytesEscritos = 0;
	int cantidadBytesTotales = 0;

	char* arrayBloques = string_new();
	string_append(&arrayBloques,"[");

	int nro_bloque = elegir_bloque_libre(cantidadBloques);
	if (nro_bloque == -1){ printf("de la nada no hay bloques libres"); return; }
	char* pathBloque = generarNombreCompletoBloque(nro_bloque);
	FILE* archivoBloque = fopen(pathBloque,"r+");

	string_append(&arrayBloques,string_itoa(nro_bloque));
	string_append(&arrayBloques,",");

	void escribirRegistro(char* registro){
		for(int i = 0; i < string_length(registro); i++){
			if(cantidadBytesEscritos == tamanioBloques){
				fclose(archivoBloque);
				free(pathBloque);
				int nro_bloque = elegir_bloque_libre(cantidadBloques);
				if (nro_bloque == -1){ printf("de la nada no hay bloques libres"); return; }
				char* pathBloque = generarNombreCompletoBloque(nro_bloque);
				FILE* archivoBloque = fopen(pathBloque,"r+");

				string_append(&arrayBloques,string_itoa(nro_bloque));
				string_append(&arrayBloques,",");

				cantidadBytesEscritos = 0;
			}
			fputc(registro[i], archivoBloque);
			cantidadBytesEscritos++;
			cantidadBytesTotales++;
		}
	}

	list_iterate(registrosDeParticion,(void*)escribirRegistro);
	fclose(archivoBloque);
	free(pathBloque);

	char* arrayBloquesFinalizado = 	string_substring_until(arrayBloques,string_length(arrayBloques)-1);
	string_append(&arrayBloquesFinalizado,"]");

	char* pathParticion = generarNombreCompletoParticion(numeroParticion,tabla);
	t_config* archivoParticion = config_create(pathParticion);

	char* sizeTotal = string_itoa(cantidadBytesTotales);

	config_set_value(archivoParticion,"BLOCKS", arrayBloquesFinalizado);
	config_set_value(archivoParticion,"SIZE", sizeTotal);

	free(sizeTotal);
	free(arrayBloques);
	free(arrayBloquesFinalizado);
	config_destroy(archivoParticion);


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

void string_append_char(char** string, char c){
	char str[2];
	str[0] =c;
	str[1]='\0';
	string_append(string,str);
}

t_list* traerRegistrosBloques(char** bloques){

	t_list* registros = list_create();
	char* registroActual = string_new();

//	void show(char* bloque){
//		printf("%s\n", bloque);
//	}
//	string_iterate_lines(bloques,(void*)show);
	void leerBloque(char* nombreBloque){
//		printf("bloque actual:%s\n",nombreBloque);
		FILE* archivoDeBloque = fopen(nombreBloque,"r");
		char nuevoCaracter = fgetc(archivoDeBloque);
		while(nuevoCaracter != EOF){
//			printf("caracter leido:%c\n",nuevoCaracter);
			if(nuevoCaracter == '\n'){
//				printf("termino registro\n");
				list_add(registros, stringRegistroAStruct(registroActual));
				free(registroActual);
				registroActual = string_new();
			}else{
				//printf("agrego caracter a reg:%c\n",nuevoCaracter);
				string_append_char(&registroActual,nuevoCaracter);
//				printf("registro con caracter agregado:%s\n", registroActual);
			}
			nuevoCaracter = fgetc(archivoDeBloque);
		}
		fclose(archivoDeBloque);

	}

	string_iterate_lines(bloques, (void*) leerBloque);



	//free(registroActual);
	printf("termine de leer bloques de la particion\n");
	return registros;
}

//-----------------------------------------------------
// ------------- getters paths completos --------------
//-----------------------------------------------------
char* generarNombreCompletoBloque(int nro_bloque){
	char* nombreCompleto = string_new();
	string_append(&nombreCompleto,puntoDeMontaje);
	string_append(&nombreCompleto,"Bloques/bloque");
	string_append(&nombreCompleto,string_itoa(nro_bloque));
	string_append(&nombreCompleto,".bin");
	return nombreCompleto;
}

char* generarNombreCompletoParticion(int numeroDeParticion,char* nombreTabla){
	char* nombreCompleto = string_new();
	string_append(&nombreCompleto,puntoDeMontaje);
	string_append(&nombreCompleto,"Tablas/");
	string_append(&nombreCompleto,nombreTabla);
	string_append(&nombreCompleto,"/part");
	string_append(&nombreCompleto,string_itoa(numeroDeParticion));
	string_append(&nombreCompleto,".bin");
	return nombreCompleto;
}

//-----------------------------------------------------
//--conversores array de numeroBloque --> path bloque--
//-----------------------------------------------------

char** transformarBloquesAPathBloques(char** nro_bloques){

	int cantBloques = 0;
	void contarBloques(char* unBloque){
		cantBloques += 1;
	}
	string_iterate_lines(nro_bloques,(void*) contarBloques);

	char** paths_bloques = (char**)malloc((cantBloques + 1) * (sizeof(char*)));
	int i = 0;
		void accionPorBloque(char* bloq){

			char* bloque = string_duplicate(puntoDeMontaje);
			string_append(&bloque,"Bloques/bloque");
			string_append(&bloque, bloq);
			string_append(&bloque, ".bin");
			paths_bloques[i] = string_duplicate(bloque);
			i++;
		}

	string_iterate_lines(nro_bloques,(void*)accionPorBloque);

	paths_bloques[cantBloques] = NULL;
	return paths_bloques;
}

char** transformarParticionABloques(char* tabla,int nro_particion){

	char** bloques = getBloquesParticion(tabla,nro_particion);
	return transformarBloquesAPathBloques(bloques);
}

char** transformarTemporalABloques(char* tabla,int nro_temporal){
	char** bloques = getBloquesTemporal(tabla,nro_temporal);
	return transformarBloquesAPathBloques(bloques);
}


//-----------------------------------------------------
// ---------- getters de nro Bloque en char* ----------
//-----------------------------------------------------

char** getBloquesParticion(char* tabla,int nro_particion){

	char* particion = string_duplicate(puntoDeMontaje);
	string_append(&particion, "Tablas/");
	string_append(&particion, tabla);
	string_append(&particion, "/part");
	string_append(&particion,string_itoa(nro_particion));
	string_append(&particion,".bin");

	t_config* metadataParticion = config_create(particion);
	char** bloques = config_get_array_value(metadataParticion,"BLOCKS");
	config_destroy(metadataParticion);

	return bloques;
}

char** getBloquesTemporal(char* tabla,int nro_temporal){

	char* temporal = string_duplicate(puntoDeMontaje);
	string_append(&temporal, "Tablas/");
	string_append(&temporal, tabla);
	string_append(&temporal, "/");
	string_append(&temporal, tabla);
	string_append(&temporal,string_itoa(nro_temporal));
	string_append(&temporal,".tmpc");

	t_config* metadataTemporal = config_create(temporal);
	char** bloques = config_get_array_value(metadataTemporal,"BLOCKS");
	config_destroy(metadataTemporal);

	return bloques;
}


//-----------------------------------------------------
// --------- conversores registro <--> char* ----------
//-----------------------------------------------------

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


//-----------------------------------------------------
// ------------- funciones para probar ----------------
//-----------------------------------------------------
void mostrarRegistro(t_registro* reg){
	printf("		key:%d;ts:%lu;value:%s\n",reg->key,reg->timestamp,reg->value);
}

void mostrarListaDeRegistros(t_list* listReg){
	printf("	PARTICION:\n");
	list_iterate(listReg,(void*)mostrarRegistro);
}

void mostrarListaDeListasDeRegistros(t_list* lista){
	printf("LISTA:\n");
	list_iterate(lista,(void*)mostrarListaDeRegistros);
}


void mostrarString(char* str){
	printf("		%s\n",str);
}

void mostrarListaDeStrings(t_list* listStr){
	printf("	PARTICION:\n");
	list_iterate(listStr,(void*)mostrarString);
}

void mostrarListaDeListasDeStrings(t_list* lista){
	printf("LISTA:\n");
	list_iterate(lista,(void*)mostrarListaDeStrings);
}


void mostrarBytes(int* bytes){
	printf( "bytes de particion %d\n",*bytes);
}

//void eliminarListaDeStrings(t_list* list){
//	list_destroy_and_destroy_elements(list,free);
//}
