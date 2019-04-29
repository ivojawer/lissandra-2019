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

int devolverSoloRequest(char* request) {
	char** requestYParametros = string_split(request, " ");
	int requestEnInt = queRequestEs(requestYParametros[0]);

	liberarArrayDeStrings(requestYParametros);

	return requestEnInt;

}

char* devolverSoloParametros(char* request) {
	char** requestYParametros = string_n_split(request, 2, " ");
	char* parametros = string_duplicate(requestYParametros[1]);

	liberarArrayDeStrings(requestYParametros);

	return parametros;
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

int esUnaRequestValida(char* request, char* parametro) {

	int requestEnInt = queRequestEs(request);

	if (requestEnInt == -1) { //No es una request
		return 0;
	}

	return esUnParametroValido(requestEnInt, parametro);

}

void* empaquetarRequest(char* request) {
	void* paquete;

	int requestEnInt = devolverSoloRequest(request);

	char* parametrosJuntos = devolverSoloParametros(request);

	char ** parametros = string_split(parametrosJuntos, " ");

	if (requestEnInt == SELECT) {

		char * nombreTabla = parametros[0];

		int tamanioNombreTabla = strlen(nombreTabla) + 1;

		int key = atoi(parametros[1]);

		paquete = malloc(
				sizeof(int) + sizeof(int) + tamanioNombreTabla + sizeof(int));

		memcpy(paquete, &requestEnInt, sizeof(int));
		memcpy(paquete + sizeof(int), &tamanioNombreTabla, sizeof(int));
		memcpy(paquete + sizeof(int) + sizeof(int), nombreTabla,
				tamanioNombreTabla);
		memcpy(paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla, &key,
				sizeof(int));

	}

	else if (requestEnInt == INSERT) {

		char * nombreTabla = parametros[0];

		int tamanioNombreTabla = strlen(nombreTabla) + 1;

		int key = atoi(parametros[1]);

		char * value = parametros[2];

		int tamanioValue = strlen(value) + 1;

		int timestamp = atoi(parametros[3]); //Ver si esto tiene que ser otra cosa que no sea int

		paquete = malloc(
				sizeof(int) + sizeof(int) + tamanioNombreTabla + sizeof(int)
						+ sizeof(int) + tamanioValue + sizeof(int));

		memcpy(paquete, &requestEnInt, sizeof(int));
		memcpy(paquete + sizeof(int), &tamanioNombreTabla, sizeof(int));
		memcpy(paquete + sizeof(int) + sizeof(int), nombreTabla,
				tamanioNombreTabla);
		memcpy(paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla, &key,
				sizeof(int));
		memcpy(
				paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla
						+ sizeof(int), &tamanioValue, sizeof(int));
		memcpy(
				paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla
						+ sizeof(int) + sizeof(int), value, tamanioValue);
		memcpy(
				paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla
						+ sizeof(int) + sizeof(int) + tamanioValue, &timestamp,
				sizeof(int));

	}

	else if (requestEnInt == CREATE) {

		char * nombreTabla = parametros[0];

		int tamanioNombreTabla = strlen(nombreTabla) + 1;

		int consistencia = queConsistenciaEs(parametros[1]);

		int particiones = atoi(parametros[2]);

		int compactTime = atoi(parametros[3]);

		paquete = malloc(
				sizeof(int) + sizeof(int) + tamanioNombreTabla + sizeof(int)
						+ sizeof(int) + sizeof(int));

		memcpy(paquete, &requestEnInt, sizeof(int));
		memcpy(paquete + sizeof(int), &tamanioNombreTabla, sizeof(int));
		memcpy(paquete + sizeof(int) + sizeof(int), nombreTabla,
				tamanioNombreTabla);
		memcpy(paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla,
				&consistencia, sizeof(int));

		memcpy(
				paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla
						+ sizeof(int), &particiones, sizeof(int));

		memcpy(
				paquete + sizeof(int) + sizeof(int) + tamanioNombreTabla
						+ sizeof(int) + sizeof(int), &compactTime, sizeof(int));
	}

	else if (requestEnInt == DESCRIBE) {
		char ** requestYParametro = string_split(request, " ");

		if (requestYParametro[1] == NULL) {
			int cero = 0;
			paquete = malloc(sizeof(int) + sizeof(int));

			memcpy(paquete, &requestEnInt, sizeof(int));
			memcpy(paquete + sizeof(int), &cero, sizeof(int));
		}

		else {

			char * nombreTabla = parametros[0];

			int tamanioNombreTabla = strlen(nombreTabla) + 1;

			paquete = malloc(sizeof(int) + sizeof(int) + tamanioNombreTabla);

			memcpy(paquete, &requestEnInt, sizeof(int));
			memcpy(paquete + sizeof(int), &tamanioNombreTabla, sizeof(int));
			memcpy(paquete + sizeof(int) + sizeof(int), nombreTabla,
					tamanioNombreTabla);

		}

		liberarArrayDeStrings(requestYParametro);

	}

	else if (requestEnInt == DROP) {

		char * nombreTabla = parametros[0];

		int tamanioNombreTabla = strlen(nombreTabla) + 1;

		paquete = malloc(sizeof(int) + sizeof(int) + tamanioNombreTabla);

		memcpy(paquete, &requestEnInt, sizeof(int));
		memcpy(paquete + sizeof(int), &tamanioNombreTabla, sizeof(int));
		memcpy(paquete + sizeof(int) + sizeof(int), nombreTabla,
				tamanioNombreTabla);

	}

	else if (requestEnInt == JOURNAL) {
		paquete = malloc(sizeof(int));
		memcpy(paquete, &requestEnInt, sizeof(int));
	}

	liberarArrayDeStrings(parametros);
	free(parametrosJuntos);
	return paquete;

}
