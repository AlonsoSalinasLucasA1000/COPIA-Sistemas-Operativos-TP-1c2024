#ifndef SERVERMEMORIA_H_
#define SERVERMEMORIA_H_
#include <commons/config.h>
#include "../../utils/src/functions/server.h"


int iniciar_servidor(void);


void iterator(char* value);

/*
void iterator(char* value) {
	log_info(logger,"%s", value);
}
*/

#endif /* SERVERMEMORIA_H_ */