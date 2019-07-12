#include "gossiping.h"
extern t_log* logger;
extern sem_t sem_gossiping;
extern sem_t sem_cargarSeeds;
extern sem_t sem_refreshConfig;
t_list* seedsConocidas;
t_list* tablaGossiping;
extern char* dirConfig;
extern int sleepGossiping;

void hacerGossipingAutomatico() {

	cargarSeedsIniciales();

	tablaGossiping = list_create();

	gossiping(); //Gossip inicial

	while (1) {

		sem_wait(&sem_refreshConfig);
		int sleepMilisegundos = sleepGossiping/1000;
		sem_post(&sem_refreshConfig);

		sleep(sleepMilisegundos);

		gossiping();

	}
}

void gossiping() {

	sem_wait(&sem_gossiping);

	for (int i = 0; i < list_size(tablaGossiping); i++) {
		memoriaGossip* unaMemoria = list_get(tablaGossiping, i);

		int resultado = enviarYRecibirSeeds(unaMemoria);

		if (resultado == -1) {
			i--; //Se borro la memoria de la lista, por lo que el contador tiene que quedarse en la misma posicion para no saltearse una memoria.
		}

	}

	tratarDeConectarseASeeds();

	sem_post(&sem_gossiping);
}

void sacarMemoriaDeTablaGossip(memoriaGossip* unaMemoria) {

	int esLaMismaMemoria(memoriaGossip* otraMemoria) {
		if (otraMemoria->nombre == unaMemoria->nombre) {
			return 1;
		}
		return 0;
	}

	sem_wait(&sem_gossiping);
	int* index = list_find(tablaGossiping, (void*) esLaMismaMemoria);

	list_remove(tablaGossiping, *index);

	close(unaMemoria->elSocket);

	free(unaMemoria);
	sem_post(&sem_gossiping);

	//No liberar la seed, es el mismo seed* que el de la lista de seedsConocidas.

}

int enviarYRecibirSeeds(memoriaGossip* memoriaDestino) {

	int seedEstaConectada(seed* unaSeed) {

		return !seedNoEstaConectada(unaSeed);
	}

	int seedNoExiste(seed* unaSeed) {
		int tienenLaMismaSeed(seed* otraSeed) {
			return esLaMismaSeed(unaSeed, otraSeed);
		}

		return !list_any_satisfy(seedsConocidas, (void*) tienenLaMismaSeed);
	}

	int seedExiste(seed* unaSeed) {
		int tienenLaMismaSeed(seed* otraSeed) {
			return esLaMismaSeed(unaSeed, otraSeed);
		}

		return list_any_satisfy(seedsConocidas, (void*) tienenLaMismaSeed);
	}

	sem_wait(&sem_cargarSeeds);

	t_list* seedsConectadas = list_filter(seedsConocidas,
			(void*) seedEstaConectada);

	enviarSeedsConHeader(memoriaDestino->elSocket, seedsConectadas, GOSSIPING);

	list_destroy(seedsConectadas);

	int operacion = recibirInt(memoriaDestino->elSocket, logger);

	t_list* seedsRecibidas = recibirSeeds(memoriaDestino->elSocket, logger);

	if (operacion != GOSSIPING) {
		sacarMemoriaDeTablaGossip(memoriaDestino);

		list_destroy(seedsRecibidas);

		sem_post(&sem_cargarSeeds);

		return -1;
	}

	if (list_size(seedsRecibidas) != 0) {

		seed* seedPrueba = list_get(seedsRecibidas, 0);
		if (seedPrueba->puerto == -1) {
			sacarMemoriaDeTablaGossip(memoriaDestino);

			free(seedPrueba);
			list_destroy(seedsRecibidas);
			sem_post(&sem_cargarSeeds);

			return -1;
		}
	} else {
		list_destroy(seedsRecibidas);
		sem_post(&sem_cargarSeeds);
		return 1; //Seeds vacias, no hay nada que hacer aca muchachos
	}
	t_list* seedsNuevas = list_filter(seedsRecibidas, (void*) seedNoExiste);

	list_add_all(seedsConocidas, seedsNuevas);

	sem_post(&sem_cargarSeeds);

	t_list* seedsALiberar = list_filter(seedsRecibidas, (void*) seedExiste);

	list_destroy_and_destroy_elements(seedsALiberar,(void*) liberarSeed);

	list_destroy(seedsRecibidas);

	return 1;
}

int esLaMismaSeed(seed* unaSeed, seed* otraSeed) {

	if (unaSeed == NULL || otraSeed == NULL) {
		return 0;
	}

	if ((unaSeed->puerto == otraSeed->puerto)
			&& !strcmp(unaSeed->ip, otraSeed->ip)) {
		return 1;
	}
	return 0;
}

void liberarSeed(seed* seedALiberar)
{
	if(seedALiberar->ip != NULL)
	{
		free(seedALiberar->ip);
	}
	free(seedALiberar);
}

int seedNoEstaConectada(seed* unaSeed) {

	int tienenLaMismaSeed(memoriaGossip* memoriaConectada) {

		return esLaMismaSeed(unaSeed, memoriaConectada->laSeed);
	}

	return !list_any_satisfy(tablaGossiping, (void*) tienenLaMismaSeed); //Si esta en la tabla de gossiping esta conectada

}

void tratarDeConectarseASeeds() {

	t_list* seedsDesconectadas = list_filter(seedsConocidas,
			(void*) seedNoEstaConectada);

	while (list_size(seedsDesconectadas) != 0) {
		seed* seedAConectarse = list_remove(seedsDesconectadas, 0);
		conectarseAOtraMemoria(seedAConectarse);
	}

	list_destroy(seedsDesconectadas);

}

void cargarSeedsIniciales() {
	t_config* config = config_create(dirConfig);
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

