#include "requests.h"

char *palabrasReservadas[9] = { "SELECT", "INSERT", "CREATE", "DESCRIBE",
		"DROP", "JOURNAL", "ADD", "RUN", "METRICS" }; //Las palabras estan ordenadas de forma tal que coincide su indice con su macro

int queRequestEs(char* palabra) {

	for (int i = 0; i < 9; i++) {

		if (!strcmp(palabra, palabrasReservadas[i])) {
			return i;
		}
	}
	return -1;
}

void liberarArrayDeStrings(char** array) {
	for (int i = 0; array[i] != NULL; i++) {
		free(array[i]);
	}
	free(array);
}

int esUnNumero(char* string) {
	if ((!atoi(string) && strcmp(string, "0"))
			|| string_contains(string, " ")) { //Si contiene espacios lo considera un numero
		return 0;
	}
	return 1;
}

int queConsistenciaEs(char* string)

{
	if (!strcmp(string, "SC")) {
		return SC;
	}

	if (!strcmp(string, "SHC")) {
		return SHC;
	}

	if (!strcmp(string, "EC")) {
		return EC;
	}
	return -1;
}

int esUnParametroValido(int request, char* parametro) {

	if (parametro == NULL && request != JOURNAL && request != DESCRIBE) {
		return 0; //Se aborta la mision porque si no se rompe everything
	}

	switch (request) {

	case SELECT:
		; //SELECT [NOMBRE_TABLA] [KEY] (int)
		{

			char**parametros = string_split(parametro, " "); //Se separa

			int resultado = 1;

			if (parametros[0] == NULL || parametros[1] == NULL
					|| parametros[2] != NULL) //Hay solo 2 parametros no vacios?
			{
				resultado = 0;
			}

			else if (!esUnNumero(parametros[1])) //El segundo parametro es un int?
					{
				resultado = 0;
			}

			liberarArrayDeStrings(parametros);

			return resultado;
		}

	case INSERT: //INSERT [NOMBRE_TABLA] [KEY] “[VALUE]” [Timestamp]

		;
		{

			char**parametros = string_n_split(parametro, 3, " "); //Se separa en 3 parametros

			int resultado = 1;

			if (parametros[0] == NULL || parametros[1] == NULL
					|| parametros[2] == NULL) {
				resultado = 0;
			}

			else if (!esUnNumero(parametros[1])) //El segundo parametro es un int?
					{
				resultado = 0;
			}

			else if (!string_starts_with(parametros[2], "\"")) //Lo que queda empieza con " ?
					{
				resultado = 0;
			}

			else {
				char* nuevoParametros2 = string_duplicate(
						string_substring_from(parametros[2], 1)); // [Value]" [Timestamp] --El string_duplicate es por si acaso :)

				if (nuevoParametros2 == NULL) //Si esta vacio
				{
					resultado = 0;
				}

				else if (!string_contains((nuevoParametros2), "\"")) //Si no contiene un "
						{
					resultado = 0;
				}

				else if (string_starts_with((nuevoParametros2), "\"")) //Si empieza con un " (esto se aclara por un tema de las commons)
						{
					resultado = 0;
				}

				else {
					char** valueYTimestamp = string_n_split(nuevoParametros2, 2,
							"\""); // [Value] y [Timestamp]

					if (string_is_empty(valueYTimestamp[0])) //Si el value esta vacio
							{
						resultado = 0;
					}

					else if (!(valueYTimestamp[1] == NULL)) //Si el timestamp no esta vacio
					{
						string_trim(&valueYTimestamp[1]); //Se le saca los espacios vacios
						if (!esUnNumero(valueYTimestamp[1])) //Si no es un numero
								{
							resultado = 0;
						}
					}

					if (!(valueYTimestamp[1] == NULL)) {
						free(valueYTimestamp[1]);
					}
					if (!(valueYTimestamp[0] == NULL)) {
						free(valueYTimestamp[0]);
					}

				}

				if (!(nuevoParametros2 == NULL)) {
					free(nuevoParametros2);
				}

			}

			liberarArrayDeStrings(parametros);
			return resultado;
		}

	case CREATE:
		; //CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]

		{

			char**parametros = string_split(parametro, " "); //Se separa

			int resultado = 1;

			if (parametros[0] == NULL //Hay solo 4 parametros no vacios?
			|| parametros[1] == NULL || parametros[2] == NULL
					|| parametros[3] == NULL || parametros[4] != NULL) {
				resultado = 0;
			}

			else if (queConsistenciaEs(parametros[1]) == -1) //El segundo parametro una de las consistencias?
					{
				resultado = 0;
			}

			else if (!esUnNumero(parametros[2])) //El tercer parametro es un int?
					{
				resultado = 0;
			}

			else if (!esUnNumero(parametros[3])) //El cuarto parametro es un int?
					{
				resultado = 0;
			}

			liberarArrayDeStrings(parametros);

			return resultado;
		}
	case DESCRIBE: //DESCRIBE [NOMBRE_TABLA]

		;
		return 1; //El parametro es opcional y no tiene formato especifico

	case DROP: //DROP [NOMBRE_TABLA]

		if (parametro == NULL) { //Hay un parametro?

			return 0;

		}

		return 1;

	case JOURNAL: //JOURNAL

		if (parametro == NULL) { //Hay un parametro?

			return 1; //No tiene que tener parametro

		}

		return 0;

	case ADD: //ADD MEMORY [NÚMERO] TO [CRITERIO]

		;
		{
			char**parametros = string_split(parametro, " "); //Se separa

			int resultado = 1;

			if (parametros[0] == NULL //Hay solo  4 parametros no vacios?
			|| parametros[1] == NULL || parametros[2] == NULL
					|| parametros[3] == NULL || parametros[4] != NULL) {
				resultado = 0;
			}

			else if (strcmp(parametros[0], "MEMORY")) //El primer parametro es "MEMORY"?
					{
				resultado = 0;
			}

			else if (!esUnNumero(parametros[1])) //El segundo parametro es un int?
					{
				resultado = 0;
			} else if (strcmp(parametros[2], "TO")) //El tercer parametro es "TO"?
					{
				resultado = 0;
			}

			else if (!queConsistenciaEs(parametros[3]) + 1) //El cuarto parametro una de las consistencias?
					{
				resultado = 0;
			}

			liberarArrayDeStrings(parametros);

			return resultado;
		}

	case RUN: // RUN [path]
		;
		{
			if (parametro == NULL) { //Hay un parametro?

				return 0;

			}

			return 1;
		}

	case METRICS: {

		if (parametro == NULL) { //Hay un parametro?

			return 1; //No tiene que tener parametro

		}

		return 0;
	}

	}
	return -1;

}

int esUnaRequestValida(char* requestEnString) {

	char** requestYParametros = string_n_split(requestEnString, 2, " ");

	int requestEnInt = queRequestEs(requestYParametros[0]);

	if (requestEnInt == -1) { //No es una request
		return 0;
	}

	if (esUnParametroValido(requestEnInt, requestYParametros[1])) {
		liberarArrayDeStrings(requestYParametros);
		return 1;
	}

	else {
		liberarArrayDeStrings(requestYParametros);
		return 0;
	}

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

int esDescribeGlobal(request* request) {
	if (request->requestEnInt == DESCRIBE
			&& !strcmp(request->parametros, " ")) {
		return 1;
	}
	return 0;
}

void liberarRequest(request* request) {
	free(request->parametros);
	free(request);
}
