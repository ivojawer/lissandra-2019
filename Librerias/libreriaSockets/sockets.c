#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>//pasarlo a un .h
#include <sys/socket.h>

int enviarMensaje(char mensaje[]){
struct sockaddr_in direccionServidor;
	direccionServidor.sin_family =AF_INET;
	direccionServidor.sin_addr.s_addr =inet_addr("127.0.0.1");
	direccionServidor.sin_port =htons(4444);


	int cliente =socket(AF_INET, SOCK_STREAM,0);
	if(connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor)) != 0){
		perror("No me pude conectar");
		return -1;
	}


	scanf("%s",mensaje);
	send(cliente,mensaje,strlen(mensaje),0);
	return 1;

}
