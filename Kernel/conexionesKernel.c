#include "funcionesKernel.h"

void conexiones()
{
	//        Creo servidor
		struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = INADDR_ANY;
		direccionServidor.sin_port = htons(4446);

		int servidor = socket(AF_INET,SOCK_STREAM,0);

		int activado = 1;
		setsockopt(servidor,SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado));

		if(bind(servidor,(void*)&direccionServidor,sizeof(direccionServidor)) != 0){
			perror("Fallo el bind");
			exit; //se podria cambiar a un int si se necesta manejo de errores.
		}
		printf("Estoy escuchando\n");
		listen(servidor,100);



		//		Acepto Cliente

		//struct sockaddr_in direccionCliente;
		//en el tutorial dice que le pase un puntero a estas cosas pero no funca
		//unsigned int tamanioDireccion;


		int cliente= accept(servidor,(void*) NULL,NULL);
		printf("Recibi una conexion en %d\n",cliente);

		char* buffer = malloc(1000);

		while(1){
			int bytesRecibidos =recv(cliente,buffer,1000,0);
			if(bytesRecibidos<=0){
				perror("Desconeccion o error de cliente");
				exit;
			}
			buffer[bytesRecibidos] ='\0';
			printf("Me llegaron %d bytes con %s\n",bytesRecibidos,buffer);
			messageHandler(buffer);

		}
		free(buffer);


}


void messageHandler(char* mensaje){
		printf("Me mandaron algo!!!");
}
