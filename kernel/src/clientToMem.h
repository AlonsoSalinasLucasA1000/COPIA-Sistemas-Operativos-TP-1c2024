#ifndef CLIENTTOMEM_H_
#define CLIENTTOMEM_H_

/* Incluidos en `#include "../../utils/src/functions/client.h"`
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
*/
#include<commons/config.h>
#include<readline/readline.h>

#include "../../utils/src/functions/client.h"

int clientToMem(void);
t_log* iniciar_logger2(void);
t_config* iniciar_config2(void);
void leer_consola2(t_log*);
void paquete2(int);
void terminar_programa2(int, t_log*, t_config*);

#endif /* CLIENTTOMEM_H_ */