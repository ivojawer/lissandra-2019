#include "gossiping.h"
extern t_log* logger;
t_list* seedsConocidas;
t_list* tablaGossiping;

void gossiping() {

	cargarSeedsIniciales();

	t_list* tablaGossiping = list_create();

	while (1) {

		t_config* config = config_create(DIRCONFIG);

		sleep(config_get_int_value(config, "RETARDO_GOSSIPING"));

		config_destroy(config);

		for (int i = 0; i<list_size(tablaGossiping);i++)
		{
			memoriaGossip* unaMemoria = list_get(tablaGossiping,i);

			int resultado = enviarYRecibirSeeds(unaMemoria);

			if(resultado == -1)
			{
				i--; //Se borro la memoria de la lista, por lo que el contador tiene que quedarse en la misma posicion para no saltearse una memoria.
			}

		}

		tratarDeConectarseASeeds();
	}
}

void sacarMemoriaDeTablaGossip(memoriaGossip* unaMemoria) {

	int esLaMismaMemoria(memoriaGossip* otraMemoria)
	{
		if (otraMemoria->nombre == unaMemoria->nombre){
			return 1;
		}
		return 0;
	}

	int* index = list_find(tablaGossiping,(void*) esLaMismaMemoria);

	list_remove(tablaGossiping,*index);

	close(unaMemoria->elSocket);

	free(unaMemoria);

	//No liberar la seed, es la misma seed que la de la lista de seedsConocidas.

}

int enviarYRecibirSeeds(memoriaGossip* memoriaDestino) {


	int seedEstaConectada(seed* unaSeed) {

		return !seedNoEstaConectada(unaSeed);
	}

	int seedNoExiste(seed* unaSeed)
	{
		int tienenLaMismaSeed(seed* otraSeed)
		{
			return esLaMismaSeed(unaSeed,otraSeed);
		}

		return !list_any_satisfy(seedsConocidas, (void*) tienenLaMismaSeed);
	}


	t_list* seedsConectadas = list_filter(seedsConocidas,
			(void*) seedEstaConectada);

	enviarSeedsConHeader(memoriaDestino->elSocket, seedsConectadas, GOSSIPING);

	list_destroy(seedsConectadas);

	int operacion = recibirInt(memoriaDestino->elSocket, logger);

	t_list* seedsRecibidas = recibirSeeds(memoriaDestino->elSocket, logger);

	if (operacion != GOSSIPING || list_size(seedsRecibidas) == 0) {
		sacarMemoriaDeTablaGossip(memoriaDestino);
		return -1 ;
	}

	t_list* seedsNuevas = list_filter(seedsRecibidas, (void*) seedNoExiste);

	list_destroy(seedsRecibidas); //TODO: Liberar de esta lista las seeds que ya existian.

	list_add_all(seedsConocidas,seedsNuevas);

	return 1;

}

int esLaMismaSeed(seed* unaSeed, seed* otraSeed) {
	if ((unaSeed->puerto == otraSeed-> puerto)
			&& !strcmp(unaSeed->ip, otraSeed->ip)) {
		return 1;
	}
	return 0;
}

int seedNoEstaConectada(seed* unaSeed) {

	int tienenLaMismaSeed(memoriaGossip* memoriaConectada) {
		return esLaMismaSeed (unaSeed,memoriaConectada->laSeed);
	}

	return !list_any_satisfy(tablaGossiping, (void*) tienenLaMismaSeed); //Si esta en la tabla de gossiping esta conectada

}

void tratarDeConectarseASeeds() {

	t_list* seedsDesconectadas = list_filter(seedsConocidas,
			(void*) seedNoEstaConectada);

	while(list_size(seedsDesconectadas) != 0)
	{
		seed* seedAConectarse = list_remove(seedsDesconectadas,0);
		conectarseAOtraMemoria(seedAConectarse);
	}

	list_destroy(seedsDesconectadas);

}

void cargarSeedsIniciales() {
	t_config* config = config_create(DIRCONFIG);
	seedsConocidas = list_create();
	char** seedsPORTDeConfig = config_get_array_value(config, "PUERTO_SEEDS");
	char** seedsIPDeConfig = config_get_array_value(config, "IP_SEEDS");
	agregarIPsYPORTsALista(seedsIPDeConfig, seedsPORTDeConfig);
	mostrarSeeds();
	config_destroy(config);
}

/*decidi recorrer ambos arrays al mismo tiempo sabiendo que si hay un problema en el .config se va a romper
 la funcion, pero esto ya iba a pasar solo que seria mas facil saber donde esta el problema, ademas asi puedo manejar
 las seeds como structs en vez de dos listas*/

void agregarIPsYPORTsALista(char** seedsIPDeConfig, char** seedsPORTDeConfig) {
	int index = 0;
	char* port = seedsPORTDeConfig[index];
	char* ip = seedsIPDeConfig[index];
	while (port != NULL && ip != NULL) {
		agregarSeed(ip, atoi(port)); //el puerto lo transformo directamente aca
		index++;
		port = seedsPORTDeConfig[index];
		ip = seedsIPDeConfig[index];
	}
}

void agregarSeed(char* ip, int port) {
	seed* nuevaSeed = malloc(sizeof(seed));
	nuevaSeed->puerto = port;
	nuevaSeed->ip = string_duplicate(ip);
	list_add(seedsConocidas, nuevaSeed);
}

//esto es para pruebas
void mostrarSeeds() {
	list_iterate(seedsConocidas, (void*) mostrarSeed);
}

void mostrarSeed(seed* miSeed) {
	printf("Seed: Puerto: %d - ip: %s\n", miSeed->puerto, miSeed->ip);
}

