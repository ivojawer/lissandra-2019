
#include "gossiping.h"

extern t_config* config;
t_list* seeds;

void gossiping(){

	cargarSeedsIniciales();


	while(1){




		sleep(config_get_int_value(config,"RETARDO_GOSSIPING"));
	}
}




void cargarSeedsIniciales(){
	seeds= list_create();
	char** seedsPORTDeConfig = config_get_array_value(config,"PUERTO_SEEDS");
	char** seedsIPDeConfig = config_get_array_value(config,"IP_SEEDS");
	agregarIPsYPORTsALista(seedsIPDeConfig,seedsPORTDeConfig);
	mostrarSeeds();
}

/*decidi recorrer ambos arrays al mismo tiempo sabiendo que si hay un problema en el .config se va a romper
 la funcion, pero esto ya iba a pasar solo que seria mas facil saber donde esta el problema, ademas asi puedo manejar
 las seeds como structs en vez de dos listas*/
void agregarIPsYPORTsALista(char** seedsIPDeConfig,char** seedsPORTDeConfig){
	int index = 0;
	char* port = seedsPORTDeConfig[index];
	char* ip = seedsIPDeConfig[index];
	while(port != NULL && ip != NULL){
		agregarSeed(ip, atoi(port));//el puerto lo transformo directamente aca
		index++;
		port = seedsPORTDeConfig[index];
		ip = seedsIPDeConfig[index];
	}
}

void agregarSeed(char* ip, int port){
	seed* nuevaSeed = malloc(sizeof(seed));
	nuevaSeed ->puerto = port;
	nuevaSeed->ip=malloc(string_length(ip));
	strcpy(nuevaSeed->ip,ip);
	list_add(seeds,nuevaSeed);
}

//esto es para pruebas
void mostrarSeeds(){
	list_iterate(seeds,(void*)mostrarSeed);
}

void mostrarSeed(seed* miSeed){
	printf("seed: puerto:%d - ip:%s\n",miSeed->puerto,miSeed->ip);
}

