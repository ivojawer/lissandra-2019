#include "requests.h"

t_config*config;
t_log* logger;

int main() {

	logger = log_create("kernel.log", "kernel", 0, 0);
	config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/kernel.config");

	char* lectura;

	lectura = readline("Dame una request: ");

	char** requestYParametros = string_n_split(lectura, 2, " ");

	char* requestEnString = requestYParametros[0];
	char* parametros = requestYParametros[1];

	int requestEnInt = queRequestEs(requestEnString);

	if (!( requestEnInt + 1)) //El +1 es para que tome el -1 como 0 y el request 0 como 1
	{
		printf("No es una request valida \n");

		//free(parametros);
		//free(requestEnString);
		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);
		return -1;
	}
	printf("%s%s", "Me diste la request ", requestEnString);

	printf("%s%s", " con el parametro ", parametros);



	if(esUnParametroValido(requestEnInt, parametros))
	{
		printf(", lo cual es valido\n");
		//free(parametros);
		//free(requestEnString);
		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);

		return 1;
	}


	printf(", lo cual es invalido\n");

	//free(parametros);
	//free(requestEnString);
	free(requestYParametros[1]);
	free(requestYParametros[0]);
	free(requestYParametros);
	free(lectura);



	return 1;
}
