#include "funcionesKernel.h"

extern t_list* listaMemorias;
extern sem_t sem_conexiones;
extern t_config* config;

void conectarseAMemoria(int puerto, char* ip) {

	memoriaEnLista* nuevaMemoria = malloc(sizeof(memoriaEnLista));

	nuevaMemoria->socket = crearConexion(puerto,ip);

	enviarIntConHeader(0,KERNEL,nuevaMemoria->socket);

	int* handshake = recibirIntConHeader(nuevaMemoria->socket);


	if (handshake[0] != MEMORIA)
	{
		return; //Agregar despues
	}

	nuevaMemoria->nombre= handshake[1];

	int* consistencias = malloc(sizeof(int)*3); // Delegar esto

	consistencias[0] = 0;
	consistencias[1] = 0;
	consistencias[2] = 0;

	nuevaMemoria->consistencias = consistencias;

	free(handshake);

	list_add(listaMemorias,nuevaMemoria);

}


void conexionPrincipal()
{
	char* ipMemoriaPrincipal = config_get_string_value(config,"IP_MEMORIA");
	int puerto = config_get_int_value(config,"PUERTO_MEMORIA");
	conectarseAMemoria(puerto,ipMemoriaPrincipal);

	memoriaEnLista* memoriaPrincipal = list_get(listaMemorias,0);

	while(1)
	{

	}
}



