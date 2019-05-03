#include "conexionesMem.h"

extern t_log* logger;
int socketLFSAMem;

void conexiones() {

	socketLFSAMem = crearConexion(4444);//conexion con LFS

	//        Creo servidor
	int puerto = 35666; //sale de config
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Fallo el bind");
		return; //se podria cambiar a un int si se necesta manejo de errores.
	}
	log_info(logger, "Estoy escuchando en puerto: %d\n", puerto);
	listen(servidor, 100);

	//		Acepto Cliente

	//struct sockaddr_in direccionCliente;
	//en el tutorial dice que le pase un puntero a estas cosas pero no funca
	//unsigned int tamanioDireccion;

	int cliente = accept(servidor, (void*) NULL, NULL);
	log_info(logger, "Recibi una conexion en %d\n", cliente);

	char* buffer = malloc(1000);

	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("Desconexion o error de cliente");
			return;
		}
		buffer[bytesRecibidos] = '\0';
		//log_info(logger,"Me llegaron %d bytes con %s\n",bytesRecibidos,buffer);z

		messageHandler(buffer,bytesRecibidos); //deberia ser del kernel, no?
	}
	free(buffer);

}

void messageHandler(char* mensaje, int tamanioMensaje) {
	log_info(logger, "Me mandaron algo: %s\n", mensaje);

	//printf("Me mandaron algo!!!");
}
