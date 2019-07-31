#include "funcionesChecker.h"

void consola() {


	while (1) {
		char *lectura = readline("Script a checkar: ");
		add_history(lectura);

		if (string_is_empty(lectura)) {
			printf("no\n");
			continue;
		}



		checkearScript(lectura);

		free(lectura);



	}


}

