#include "funcionesKernel.h"

t_log* logger;
t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaTablas;
t_list* tiemposInsert;
t_list* tiemposSelect;
t_list* memorias;
t_list* seedsMemorias;
t_list* seedsSiendoCreadas;
int idInicial;
int quantum;
int multiprocesamiento;
int proximaMemoriaEC;
int sleepEjecucion;
int intervaloDeRefreshMetadata;
int operacionesTotales;
int retardoGossiping;
sem_t sem_multiprocesamiento;
sem_t sem_cambioId;
sem_t sem_disponibleColaREADY;
sem_t sem_tiemposInsert;
sem_t sem_tiemposSelect;
sem_t sem_refreshConfig;
sem_t sem_cambioMemoriaEC;
sem_t sem_movimientoScripts;
sem_t sem_borradoMemoria;
sem_t sem_actualizacionMetadatas;
sem_t sem_operacionesTotales;
sem_t sem_gossiping;
sem_t sem_seedSiendoCreada;
script* scriptRefreshMetadata;

int main() {

	logger = log_create("kernel.log", "kernel", 1, 0);
	t_config* config = config_create(DIRCONFIG);

	quantum = config_get_int_value(config, "QUANTUM");
	multiprocesamiento = config_get_int_value(config, "MULTIPROCESAMIENTO");
	sleepEjecucion = config_get_int_value(config, "SLEEP_EJECUCION");
	intervaloDeRefreshMetadata = config_get_int_value(config,
			"METADATA_REFRESH");
	retardoGossiping = config_get_int_value(config,
				"GOSSIP_SLEEP");

	colaNEW = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	listaEXIT = list_create();
	listaTablas = list_create();
	tiemposInsert = list_create();
	tiemposSelect = list_create();
	memorias = list_create();
	seedsMemorias = list_create();
	seedsSiendoCreadas = list_create();

	idInicial = 1000;
	proximaMemoriaEC = -1;
	operacionesTotales = 0;

	scriptRefreshMetadata = malloc(sizeof(script));

	scriptRefreshMetadata->esPorConsola = 0;
	scriptRefreshMetadata->idScript = 1;
	scriptRefreshMetadata->direccionScript = string_new();
	sem_init(&scriptRefreshMetadata->semaforoDelScript, 0, 0);

	pthread_t h_consola;
	pthread_t h_planificador;
	pthread_t h_primeraConexion;
	pthread_t h_refreshMetadatas;
	pthread_t h_refreshConfig;
	pthread_t h_metricsAutomatico;
	pthread_t h_gossiping;

	sem_init(&sem_multiprocesamiento, 0, multiprocesamiento);
	sem_init(&sem_cambioId, 0, 1);
	sem_init(&sem_disponibleColaREADY, 0, 0);
	sem_init(&sem_tiemposInsert, 0, 1);
	sem_init(&sem_tiemposSelect, 0, 1);
	sem_init(&sem_refreshConfig, 0, 1);
	sem_init(&sem_cambioMemoriaEC,0,1);
	sem_init(&sem_movimientoScripts,0,1);
	sem_init(&sem_borradoMemoria,0,1);
	sem_init(&sem_actualizacionMetadatas,0,1);
	sem_init(&sem_operacionesTotales,0,1);
	sem_init(&sem_gossiping,0,1);
	sem_init(&sem_seedSiendoCreada,0,1);

	char* ipMemoriaPrincipal = string_duplicate(config_get_string_value(config, "IP_MEMORIA"));
	int puertoMemoriaPrincipal = config_get_int_value(config, "PUERTO_MEMORIA");

	config_destroy(config);

	seed* seedPrincipal = malloc(sizeof(seed));

	seedPrincipal->ip = ipMemoriaPrincipal;
	seedPrincipal->puerto = puertoMemoriaPrincipal;

	list_add(seedsMemorias,seedPrincipal);

	pthread_create(&h_primeraConexion, NULL, (void *) conectarseAUnaMemoria,
			seedPrincipal);


	pthread_detach(h_primeraConexion);

	pthread_create(&h_planificador, NULL, (void *) planificadorREADYAEXEC,
	NULL);
	pthread_detach(h_planificador);

	pthread_create(&h_refreshMetadatas, NULL, (void *) refreshMetadatas,
	NULL);
	pthread_detach(h_refreshMetadatas);

	pthread_create(&h_refreshConfig, NULL, (void *) refreshConfig,
	NULL);
	pthread_detach(h_refreshConfig);

	pthread_create(&h_metricsAutomatico, NULL, (void *) metricsAutomatico,
		NULL);
	pthread_detach(h_metricsAutomatico);

	pthread_create(&h_gossiping, NULL, (void *) gossipingAutomatico,NULL);
	pthread_detach(h_gossiping);

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_join(h_consola, NULL);

	//Matar lo que se pueda

	log_destroy(logger);

	sem_destroy(&sem_multiprocesamiento);
	sem_destroy(&sem_cambioId);
	sem_destroy(&sem_disponibleColaREADY);
	sem_destroy(&sem_tiemposInsert);
	sem_destroy(&sem_tiemposSelect);
	sem_destroy(&sem_refreshConfig);
	sem_destroy(&sem_cambioMemoriaEC);
	sem_destroy(&sem_movimientoScripts);
	sem_destroy(&sem_borradoMemoria);
	sem_destroy(&sem_actualizacionMetadatas);
	sem_destroy(&sem_operacionesTotales);

	free(ipMemoriaPrincipal);
	free(seedPrincipal);

	matarListas();

	return 0; //Gracias tux tkm
}
