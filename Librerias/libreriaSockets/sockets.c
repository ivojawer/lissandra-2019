#include "sockets.h"

int enviarMensaje(char* mensaje,int puerto){
struct sockaddr_in direccionServidor;
	direccionServidor.sin_family =AF_INET;
	direccionServidor.sin_addr.s_addr =inet_addr("127.0.0.1");
	direccionServidor.sin_port =htons(puerto);


	int cliente =socket(AF_INET, SOCK_STREAM,0);
	if(connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor)) != 0){
		perror("No me pude conectar");
		return -1;
	}


	scanf("%s",mensaje);
	send(cliente,mensaje,strlen(mensaje),0);
	return 1;
}
