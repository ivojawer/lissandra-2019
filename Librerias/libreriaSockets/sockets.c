#include "sockets.h"

int crearConexion(int puerto){
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family =AF_INET;
	direccionServidor.sin_addr.s_addr =inet_addr("127.0.0.1");//el ip tiene que salir del config
	direccionServidor.sin_port =htons(puerto);


	int conexion =socket(AF_INET, SOCK_STREAM,0);
	if(connect(conexion,(void*) &direccionServidor,sizeof(direccionServidor)) != 0){
		perror("No me pude conectar");
		return -1;
	}
	return conexion;
}

int enviarMensaje(char* mensaje,int socket){
	send(socket,mensaje,strlen(mensaje),0);
	return 1;
}
