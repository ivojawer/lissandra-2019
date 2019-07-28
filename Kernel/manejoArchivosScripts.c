#include "funcionesKernel.h"

int existeArchivo(char* script) {
	FILE* archivo;

	char* direccion = scriptConRaiz(script);

	archivo = fopen(direccion, "r");

	if (archivo == NULL) {
		return 0;
	}

	fclose(archivo);
	free(direccion);

	return 1;
}

void limpiarBuffer(char* buffer) {
	for (int i = 0; i < MAXBUFFER; i++) {
		buffer[i] = '\n';
	}
}

int charsDeBuffer(char* buffer) {
	int caracteres = 0;
	for (; caracteres < MAXBUFFER; caracteres++) {
		if (buffer[caracteres] == '\n') {
			break;
		}
	}

	return caracteres + 1;
}

char* leerLinea(char* direccion, int lineaALeer) {

	FILE* archivo;
	archivo = fopen(direccion, "r");
	char* resultado = string_new();

	if (archivo == NULL) {

		string_append(&resultado, "fin");

		return resultado;
	}

	for (int i = 0; i <= lineaALeer; i++) {

		char buffer[MAXBUFFER];

		char* resultadoDeLeer = fgets(buffer, MAXBUFFER, archivo);

		if (resultadoDeLeer == NULL) {

			string_append(&resultado, "fin");

			free(resultadoDeLeer);

			break;

		}

		if (i == lineaALeer) {

			string_append(&resultado, resultadoDeLeer);

			char* resultadoInverso = string_reverse(resultado);

			if (resultadoInverso[0] == '\n') { //Si tiene un \n al final

				char* resultadoAux = resultado;
				resultado = string_substring_until(resultado,
						strlen(resultado) - 1); //Se saca el \n
				free(resultadoAux);
			}

			free(resultadoInverso);

		}

	}

	fclose(archivo);

	return resultado;

}
