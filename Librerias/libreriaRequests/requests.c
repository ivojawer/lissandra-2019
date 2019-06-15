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

	if (requestYParametros[1] == NULL) {
		char* parametrosRequest = string_duplicate(" ");
		requestNuevo->parametros = parametrosRequest;
	}

	else {
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

		if (!strcmp(" ", request->parametros)) {
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



void liberarRequest(request* request) {
	free(request->parametros);
	free(request);
}
