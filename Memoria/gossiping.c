#include "gossiping.h"
extern t_log* logger;
extern sem_t sem_gossiping;
//extern sem_t sem_cargarSeeds;
extern sem_t sem_refreshConfig;
t_list* seedsConocidas;
extern t_list* tablaGossiping;
extern char* dirConfig;
extern int sleepGossiping;
extern int nombreMemoria;

void hacerGossipingAutomatico() {


	tratarDeConectarseASeeds();
	gossiping(); //Gossip inicial

	while (1) {

		sem_wait(&sem_refreshConfig);
		int sleepMilisegundos = sleepGossiping/1000;
		sem_post(&sem_refreshConfig);

		sleep(2);

		gossiping();

	}
}

void gossiping() {

	sem_wait(&sem_gossiping);

	log_info(logger,"Se va a empezar a hacer gossiping.");
	tratarDeConectarseASeeds();

	for (int i = 0; i < list_size(tablaGossiping); i++) {
		memoriaGossip* unaMemoria = list_get(tablaGossiping, i);

		if (unaMemoria->nombre != nombreMemoria)
		{
			enviarSeedsConectadas(unaMemoria,GOSSIPING);
		}

	}
	log_info(logger,"Se termino el gossiping.");

	sem_post(&sem_gossiping);
}

void sacarMemoriaDeTablaGossip(memoriaGossip* unaMemoria) {

	int esLaMismaMemoria(memoriaGossip* otraMemoria) {
		if (otraMemoria->nombre == unaMemoria->nombre) {
			return 1;
		}
		return 0;
	}

	log_error(logger,"Se desconecto la memoria %i",unaMemoria->nombre);

//	int* index = list_find(tablaGossiping, (void*) esLaMismaMemoria); ESTO ME TRAICIONO

	int index = -1;
	for (int i = 0;i<list_size(tablaGossiping);i++)
	{
		memoriaGossip* otraMemoria = list_get(tablaGossiping,i);

		if(esLaMismaMemoria(otraMemoria))
		{
			index = i;
		}
	}

	if(index != -1)
	{
		list_remove(tablaGossiping, index);
	}


	close(unaMemoria->elSocket);

	free(unaMemoria);

	//No liberar la seed, es el mismo seed* que el de la lista de seedsConocidas.

}

void enviarSeedsConectadas(memoriaGossip* memoriaDestino,int tipoDeEnvio) //Sincro por afuera
{	//tipo de envio = GOSSIP o RESPUESTA
	//GOSSIP: se envia las seeds por gossip automatico
	//RESPUESTA: se envia las seeds por respuesta a un gossip de otra memoria
	int seedEstaConectada(seed* unaSeed) {

		return !seedNoEstaConectada(unaSeed);
	}

	t_list* seedsConectadas = list_filter(seedsConocidas,
				(void*) seedEstaConectada);

	enviarSeedsConHeader(memoriaDestino->elSocket, seedsConectadas, tipoDeEnvio); //Se responde con RESPUESTA

	list_destroy(seedsConectadas);

}

void agregarNuevasSeeds(t_list* seeds) { //Sincronizar por afuera

		int seedExiste(seed* unaSeed) {
			int tienenLaMismaSeed(seed* otraSeed) {
				return esLaMismaSeed(unaSeed, otraSeed);
			}

			return list_any_satisfy(seedsConocidas, (void*) tienenLaMismaSeed);
		}

		int seedNoExiste(seed* unaSeed) {

				return !seedExiste(unaSeed);
			}


	t_list* seedsALiberar = list_filter(seeds, (void*) seedExiste);

	t_list* seedsNuevas = list_filter(seeds, (void*) seedNoExiste);

	list_add_all(seedsConocidas, seedsNuevas);

	list_destroy_and_destroy_elements(seedsALiberar, (void*) liberarSeed);
}

int esLaMismaSeed(seed* unaSeed, seed* otraSeed) {

	if (unaSeed == NULL || otraSeed == NULL  || (unaSeed->ip == NULL) || (otraSeed->ip == NULL)) {
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

	mostrarSeeds();

	t_list* seedsDesconectadas = list_filter(seedsConocidas,
			(void*) seedNoEstaConectada);

	while (list_size(seedsDesconectadas) != 0) {
		seed* seedAConectarse = list_remove(seedsDesconectadas, 0);
		if(seedAConectarse->ip != NULL)
		{
			log_info(logger,"Me estoy tratando de conectar a %s:%i",seedAConectarse->ip,seedAConectarse->puerto);
					conectarseAOtraMemoria(seedAConectarse);
		}
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

