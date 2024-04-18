#ifndef SERVERMEMORIA_H_
#define SERVERMEMORIA_H_
#include <commons/config.h>
#include "../../utils/src/functions/server.h"


int iniciar_servidor(void);


void iterator(char* value);
t_log* iniciar_logger(void);
t_config* iniciar_config(void);

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_log *iniciar_logger(void)
{
    t_log *nuevo_logger;

    return nuevo_logger;
}

t_config *iniciar_config(void)
{
    t_config *nuevo_config;

    return nuevo_config;
}

#endif /* SERVERMEMORIA_H_ */