#include "requests.h"

int main() {

	char* lectura;

	lectura = readline("Dame una request: ");

	char** requestYParametros = string_n_split(lectura, 2, " ");

	char* requestEnString = requestYParametros[0];
	char* parametros = requestYParametros[1];

	int requestEnInt = esUnaRequestYCual(requestEnString);

	if (!( requestEnInt + 1)) //El +1 es para que tome el -1 como 0 y el request 0 como 1
	{
		printf("No es una request valida \n");
		return -1;
	}
	printf("%s%s", "Me diste la request ", requestEnString);

	printf("%s%s", " con el parametro ", parametros);



	if(esUnParametroValido(requestEnInt, parametros))
	{
		printf(", lo cual es valido\n");

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);

		return 1;
	}


	printf(", lo cual es invalido\n");

	//free(parametros);
	//free(requestEnString);
	free(requestYParametros[1]);
	free(requestYParametros[0]);
	free(requestYParametros);



	return 1;
}
