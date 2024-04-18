#ifndef SERVERMEMORIA_H_
#define SERVERMEMORIA_H_

/* Incluidas en `#include "../../utils/src/functions/server.h"`
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
*/
#include <commons/config.h>
#include "../../utils/src/functions/server.h"


int iniciar_servidor(void);
void iterator(char* value);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);

#endif /* SERVERMEMORIA_H_ */