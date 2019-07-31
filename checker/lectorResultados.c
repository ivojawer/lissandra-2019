#include "funcionesChecker.h"

extern int tamanioValue;
extern t_log *resultadoCheck;
extern t_log* erroresCheck;

resultado* leerLineaResultado(char* direccionScript, int lineaALeer) {

	FILE* archivo;
	int MAXBUFFER = tamanioValue + 10; //El +10 por si acaso :)
	archivo = fopen(direccionScript, "r");

	char* resultadoEnString = string_new();

	resultado* resultado = malloc(sizeof(resultado));

	if (archivo == NULL) {

		resultado->value = string_new();
		string_append(&resultado->value, "fin");

		return resultado;
	}

	for (int i = 0; i <= lineaALeer; i++) {

		char buffer[MAXBUFFER];

		char* resultadoDeLeer = fgets(buffer, MAXBUFFER, archivo);

		if (resultadoDeLeer == NULL) {

			resultado->value = string_new();

			string_append(&resultado->value, "fin");

			free(resultadoDeLeer);

			break;

		}

		if (i == lineaALeer) {

			string_append(&resultadoEnString, resultadoDeLeer);

			char* resultadoInverso = string_reverse(resultadoEnString);

			if (resultadoInverso[0] == '\n') { //Si tiene un \n al final

				char* resultadoAux = resultadoEnString;
				resultadoEnString = string_substring_until(resultadoEnString,
						strlen(resultadoEnString) - 1); //Se saca el \n
				free(resultadoAux);
			}

			free(resultadoInverso);

			char** cosasDelResultado = string_n_split(resultadoEnString, 3,
					" ");
			char* nombreTabla = string_duplicate(cosasDelResultado[0]);
			uint16_t key = atoi(cosasDelResultado[1]);
			char* valueConComillas = string_duplicate(cosasDelResultado[2]);

			char* punteroAux = valueConComillas;

			valueConComillas = string_substring_until(valueConComillas,
					strlen(valueConComillas) - 1);

			free(punteroAux);

			char* value = string_substring_from(valueConComillas, 1);

			free(valueConComillas);

			resultado->key = key;
			resultado->value = value;
			resultado->nombreTabla = nombreTabla;

			liberarArrayDeStrings(cosasDelResultado);

		}

	}

	fclose(archivo);

	return resultado;
}

char* bloquesEnString(t_list* bloques) {
	char* bloquesString = string_new();

	for (int i = 0; i < list_size(bloques); i++) {

		int* unBloque = list_get(bloques, i);

		string_append(&bloquesString, string_itoa(*unBloque));

		string_append(&bloquesString, " ");

	}

	return bloquesString;
}

void checkearScript(char* nombreScript) {
	int lineaALeer = 0;
	char* direccionScript = string_new();
	string_append(&direccionScript,
			"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/checker/resultados/");
	string_append(&direccionScript, nombreScript);
	string_append(&direccionScript, ".txt");

	while (1) {
		resultado* resultadoLectura = leerLineaResultado(direccionScript,
				lineaALeer);

		lineaALeer++;

		if(!strcmp(resultadoLectura->value,"fin")){
			free(resultadoLectura->value);
			free(resultadoLectura);
			break;
		}


		char* requestSelect = string_new();
		string_append(&requestSelect, resultadoLectura->nombreTabla);
		string_append(&requestSelect, " ");
		string_append(&requestSelect, string_itoa(resultadoLectura->key));

		resultadoSelect* resultadoSelect = rutina_select(requestSelect);
		char* punteroAux = resultadoSelect->value;
		resultadoSelect->value = string_substring_until(resultadoSelect->value,strlen(resultadoSelect->value)-1); //Se saca el \n
		free(punteroAux);

		if(resultadoSelect->value == NULL)
		{
			log_error(resultadoCheck,
								"SELECT %s %i NO SE ENCONTRO: \nEsperado: |%s|",
								resultadoLectura->nombreTabla, resultadoLectura->key,
								resultadoLectura->value);
			log_error(erroresCheck,
											"SELECT %s %i NO SE ENCONTRO: \nEsperado: |%s|",
											resultadoLectura->nombreTabla, resultadoLectura->key,
											resultadoLectura->value);
			continue;
		}

		if (!strcmp(resultadoSelect->value, resultadoLectura->value)) {
			log_info(resultadoCheck,
					"SELECT %s %i: \nEsperado: |%s| \nConseguido: |%s|\n",
					resultadoLectura->nombreTabla, resultadoLectura->key,
					resultadoLectura->value, resultadoSelect->value);
		} else {
			log_error(resultadoCheck,
					"SELECT %s %i: \nEsperado: |%s| \nConseguido: |%s|\n",
					resultadoLectura->nombreTabla, resultadoLectura->key,
					resultadoLectura->value, resultadoSelect->value);
			log_error(erroresCheck,
								"SELECT %s %i: \nEsperado: |%s| \nConseguido: |%s|\n",
								resultadoLectura->nombreTabla, resultadoLectura->key,
								resultadoLectura->value, resultadoSelect->value);
		}

		free(requestSelect);
		free(resultadoSelect->value);
		list_destroy(resultadoSelect->bloquesInicialesPotenciales);
		free(resultadoSelect);
		free(resultadoLectura->nombreTabla);
		free(resultadoLectura->value);
		free(resultadoLectura);
	}
}
