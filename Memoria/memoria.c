#include "requests.h"

t_config*config;
t_log* logger;

int main() {

	logger = log_create("memoria.log", "memoria", 0, 0);
	config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/memoria.config");

	return 1;
}
