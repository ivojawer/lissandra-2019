#include "requests.h"

//Las palabras estan ordenadas de forma tal que coincide su indice con su macro

void liberarArrayDeStrings(char** array) {
	for (int i = 0; array[i] != NULL; i++) {

		free(array[i]);

	}
	free(array);

}

request* crearStructRequest(char* requestEnString) {
	char** requestYParametros = string_n_split(requestEnString, 2, " ");

	int requestInt = queRequestEs(requestYParametros[0]);

	request* requestNuevo = malloc(sizeof(request));
	requestNuevo->requestEnInt = requestInt;

	if(requestYParametros[1] == NULL){
		char* parametrosRequest = string_duplicate(" ");
		requestNuevo->parametros = parametrosRequest;
	}else{
		char* parametrosRequest = string_duplicate(requestYParametros[1]);
		requestNuevo->parametros = parametrosRequest;
	}

	liberarArrayDeStrings(requestYParametros);

	return requestNuevo;

}

char* requestStructAString(request* request) {
	char* requestEnString = string_new();

	switch (request->requestEnInt) {
	case SELECT: {
		string_append(&requestEnString, "SELECT ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case INSERT: {
		string_append(&requestEnString, "INSERT ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case CREATE: {
		string_append(&requestEnString, "CREATE ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case DESCRIBE: {

		if (!strcmp(" ", request->parametros) || request->parametros == NULL) {
			string_append(&requestEnString, "DESCRIBE");

		} else {

			string_append(&requestEnString, "DESCRIBE ");
			string_append(&requestEnString, request->parametros);
		}
		break;
	}

	case DROP: {
		string_append(&requestEnString, "DROP ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case JOURNAL: {
		string_append(&requestEnString, "JOURNAL");
		break;
	}
	case ADD: {
		string_append(&requestEnString, "ADD ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case RUN: {
		string_append(&requestEnString, "RUN ");
		string_append(&requestEnString, request->parametros);
		break;
	}
	case METRICS: {
		string_append(&requestEnString, "METRICS");
		break;
	}
	}

	return requestEnString;
}

//retorna el indice de la primera ocurrencia del char o -1 si no aparece nunca
int str_first_index_of(char c, char* cadena) {
	int i = 0;
	while (cadena[i] != c && cadena[i] != '\0') {
		i++;
	}

	return cadena[i] == '\0' ? -1 : i;
}

//retorna el indice de la ultima ocurrencia del char o -1 si no aparece nunca
int str_last_index_of(char c, char* cadena) {
	int ultima_ocurrencia = -1;

	for (int i = 0; i < string_length(cadena); i++) {
		if (cadena[i] == c) {
			ultima_ocurrencia = i;
		}
	}

	return ultima_ocurrencia;
}

int lista_vacia(t_list *lista) {
	if (list_size(lista) == 0) {
		return 1;
	} else {
		return 0;
	}
}

void liberarRequest(request* request) {
	free(request->parametros);
	free(request);
}

char* consistenciaEnString(int consistenciaEnInt) {
	switch (consistenciaEnInt) {
	case SC: {
		return "SC";
	}
	case EC:{
		return "EC";
	}
	case SHC:{
		return "SHC";
	}

	}
	return " ";
}

void describirMetadatas(t_list* metadatas) {

	printf("\n\n--------METADATAS--------\n\n");

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* unaMetadata = list_get(metadatas, i);

		printf("Tabla: %s\n", unaMetadata->nombre);
		printf("Consistencia: %s\n", consistenciaEnString(unaMetadata->consistencia));
		printf("Particiones: %i\n", unaMetadata->particiones);
		printf("Compact time: %i\n\n", unaMetadata->compactTime);
	}
	printf("-------------------------\n\n");
}

void describirUnaMetadata (metadataTablaLFS* unaMetadata)
{
	printf("Tabla: %s\n", unaMetadata->nombre);
			printf("Consistencia: %s\n", consistenciaEnString(unaMetadata->consistencia));
			printf("Particiones: %i\n", unaMetadata->particiones);
			printf("Compact time: %i\n\n", unaMetadata->compactTime);
}

void liberarMetadata(metadataTablaLFS* unaMetadata)
{
	if(unaMetadata->nombre != NULL)
	{
		free(unaMetadata->nombre);
	}
	free(unaMetadata);
}

void liberarListaMetadatas(t_list* metadatas)
{
	while(list_size(metadatas) != 0)
	{
		metadataTablaLFS* unaMetadata = list_remove(metadatas,0);

		liberarMetadata(unaMetadata);
	}
}
