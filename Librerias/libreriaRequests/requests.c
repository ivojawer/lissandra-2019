#include "requests.h"

char *palabrasReservadas[8] = { "SELECT", "INSERT", "CREATE", "DESCRIBE",
		"DROP", "JOURNAL", "ADD", "RUN" }; //Las palabras estan ordenadas de forma tal que coincide su indice con su macro

int queRequestEs(char* palabra) {

	for (int i = 0; i < 8; i++) {

		if (!strcmp(palabra, palabrasReservadas[i])) {
			return i;
		}
	}
	return -1;
}

int esUnNumero(char* string) {
	if (!atoi(string) && strcmp(string, "0")) {
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

	if (parametro == 0 && request != JOURNAL && request != DESCRIBE) {
		return 0; //Se aborta la mision porque si no se rompe everything
	}

	switch (request) {

	case SELECT:
		; //SELECT [NOMBRE_TABLA] [KEY] (int)
		{

			char**parametros = string_n_split(parametro, 2, " "); //Se separa en 2 parametros

			int resultado = 1;

			if (parametros[0] == NULL || parametros[1] == NULL) //Hay 2 parametros no vacios?
					{
				resultado = 0;
			}

			else if (!esUnNumero(parametros[1])) //El segundo parametro es un int?
					{
				resultado = 0;
			}

			for (int i = 0; i < 2; i++) {
				free(parametros[i]);

				if (parametros[i + 1] == 0) {
					break;
				}
			}
			free(parametros);

			return resultado;
		}

	case INSERT: //INSERT [NOMBRE_TABLA] [KEY] “[VALUE]” [Timestamp]

		;

		//TODO: ver que el value puede tener espacios adentro y eso corta el split
		//TODO: el timestamp es opcional en el FS y ¿no se pone en el kernel y memoria? (preguntar esto)
		{

			char**parametros = string_n_split(parametro, 4, " "); //Se separa en 4 parametros

			int resultado = 1;

			if (parametros[0] == NULL //Hay 3 parametros seguidos no vacios? (El cuarto es opcional, puede ser vacio)
			|| parametros[1] == NULL || parametros[2] == NULL) {
				resultado = 0;
			}

			else if (!esUnNumero(parametros[1])) //El segundo parametro es un int?
					{
				resultado = 0;
			}

			else if (!string_starts_with(parametros[2], "\"")
					|| !string_ends_with(parametros[2], "\"")) //El tercer parametro empieza y termina con '"' ?
							{
				resultado = 0;
			}

			else if (parametros[3] != NULL) //Si el cuarto parametro existe...
					{
				if (!esUnNumero(parametros[3])) //Es un int?
						{
					return 0;
				}
			}

			for (int i = 0; i < 4; i++) {
				free(parametros[i]);

				if (parametros[i + 1] == 0) {
					break;
				}
			}
			free(parametros);
			return resultado;
		}

	case CREATE:
		; //CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]

		{

			char**parametros = string_n_split(parametro, 4, " "); //Se separa en 4 parametros

			int resultado = 1;

			if (parametros[0] == NULL //Hay 4 parametros no vacios?
			|| parametros[1] == NULL || parametros[2] == NULL || parametros[3] == NULL) {
				resultado = 0;
			}

			else if (!queConsistenciaEs(parametros[1]) + 1) //El segundo parametro una de las consistencias?
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

			for (int i = 0; i < 4; i++) {
				free(parametros[i]);

				if (parametros[i + 1] == 0) {
					break;
				}
			}
			free(parametros);

			return resultado;
		}
	case DESCRIBE: //DESCRIBE [NOMBRE_TABLA]

		;
		return 1; //El parametro es opcional y no tiene formato especifico

	case DROP: //DROP [NOMBRE_TABLA]

		if (parametro == 0) { //Hay un parametro?

			return 0;

		}

		return 1;

	case JOURNAL: //JOURNAL

		if (parametro == 0) { //Hay un parametro?

			return 1; //No tiene que tener parametro

		}

		return 0;

	case ADD: //ADD MEMORY [NÚMERO] TO [CRITERIO]

		;
		{
			char**parametros = string_n_split(parametro, 4, " "); //Se separa en 4 parametros

			int resultado = 1;

			if (parametros[0] == NULL //Hay 4 parametros no vacios?
			|| parametros[1] == NULL || parametros[2] == NULL || parametros[3] == NULL) {
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

			for (int i = 0; i < 4; i++) {
				free(parametros[i]);

				if (parametros[i + 1] == 0) {
					break;
				}
			}

			free(parametros);

			return resultado;
		}

	case RUN: // RUN [path]
		;
		{
			if (parametro == 0) { //Hay un parametro?

				return 0;

			}

			return 1;
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
