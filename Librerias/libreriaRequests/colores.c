#include "requests.h"

void loggearConColor(t_log* logger, char* texto, char* codigoColor)
{
	log_info(logger,"\033%s   %s       \033[0m",codigoColor,texto);
}

void loggearRojo(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;31m");
}

void loggearRojoClaro(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;31m");
}

void loggearVerde(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;32m");
}

void loggearVerdeClaro(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;32m");
}

void loggearMarron(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;33m");
}

void loggearAmarillo(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;33m");
}

void loggearAzul(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;34m");
}

void loggearAzulClaro(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;34m");
}

void loggearMagenta(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;35m");
}

void loggearMagentaClaro(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;35m");
}

void loggearCyan(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[0;36m");
}

void loggearCyanClaro(t_log* logger, char* texto)
{
	loggearConColor(logger,texto,"[1;36m");
}
