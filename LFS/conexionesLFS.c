#include "funcionesLFS.h"

#include <arpa/inet.h>//pasarlo a un .h
#include <sys/socket.h>

extern t_log* logger;
int socketLFSAMEM;
void conexiones()
{
	socketLFSAMEM = crearConexion(35666);//conexion con memoria
	//        Creo servidor
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(4444);

	int servidor = socket(AF_INET,SOCK_STREAM,0);

	int activado = 1;
	setsockopt(servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

	if(bind(servidor,(void*)&direccionServidor,sizeof(direccionServidor)) != 0){
		perror("Fallo el bind");
		exit; //se podria cambiar a un int si se necesta manejo de errores.
	}
	log_info(logger,"Estoy escuchando\n");
	listen(servidor,100);



	//		Acepto Cliente

	//struct sockaddr_in direccionCliente;
	//en el tutorial dice que le pase un puntero a estas cosas pero no funca
	//unsigned int tamanioDireccion;


	int cliente= accept(servidor,(void*) NULL,NULL);
	log_info(logger,"Recibi una conexion en %d\n",cliente);

	char* buffer = malloc(100);

	while(1){
		int bytesRecibidos =recv(cliente,buffer,100,0);
		if(bytesRecibidos<=0){
			perror("Desconeccion o error de cliente");
			exit;
		}
		buffer[bytesRecibidos] ='\0';
		messageHandler(buffer);
		//printf("Me llegaron %d bytes con %s\n",bytesRecibidos,buffer);


	}
	free(buffer);

}


void messageHandler(char* lectura){
	if (string_is_empty(lectura)) {
		log_info(logger,"No es una request valida, vuelva prontos \n");
		free(lectura);

	}

	char** requestYParametros = string_n_split(lectura, 2, " ");

	int requestEnInt = queRequestEs(requestYParametros[0]);

	if (!esUnaRequestValida(requestYParametros[0],
			requestYParametros[1]) || requestEnInt == JOURNAL
			|| requestEnInt == ADD || requestEnInt == RUN) { //Si es invalida o es una request que no vale en el LFS

		log_info(logger,"No es una request valida, vuelva prontos \n");

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);
		return;


	}

	if (requestYParametros[1] == NULL) { //Para que no rompa en el string_duplicate de funcionesLFS.c
		requestYParametros[1] = (char *) malloc(sizeof(" "));
		strcpy(requestYParametros[1], " ");
	}
	log_info(logger,"mando a ejecutar una request");
	mandarAEjecutarRequest(requestEnInt, requestYParametros[1]);
}



