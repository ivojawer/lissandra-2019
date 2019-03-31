#include "requests.h"

char *palabrasReservadas[7] = { "SELECT", "INSERT", "CREATE", "DESCRIBE",
		"DROP", "JOURNAL", "ADD" }; //Las palabras estan ordenadas de forma tal que coincide su indice con su macro

int esUnaRequestYCual(char* palabra) {

	for (int i = 0; i < 7; i++) {

		if (!strcmp(palabra, palabrasReservadas[i])) {
			return i;
		}
	}
	return -1;
}

int main() {

	char* lectura;

	lectura = readline("Dame una request: ");

	char** requestYParametros = string_n_split(lectura, 2, " ");

	char* requestEnString = requestYParametros[0];
	char* parametros = requestYParametros[1];

	if (!( esUnaRequestYCual(requestEnString) +1 ) ) //El +1 es para que tome el -1 como 0 y el request 0 como 1
	{
		printf("No es una request valida \n");
		return -1;
	}
	printf("%s%s","Me diste la request ",requestEnString);

	if (parametros == 0) //Si no hay parametro entonces el parametro tiene la direccion 0
	{
		printf(" sin ningun parametro");
		return 1;
	}

	printf("%s%s"," con el parametro ",parametros);


	return 1;
}
