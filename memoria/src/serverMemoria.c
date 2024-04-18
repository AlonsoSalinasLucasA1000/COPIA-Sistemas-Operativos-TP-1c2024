#include "serverMemoria.h"
#include <commons/config.h>

//#define destino "CPU"
#define destino "MEM"
//#define destino "KER"

int iniciar_servidor() {
	
	t_log *logger;

	logger = iniciar_logger();
	char pathLogs [27] = ".//src/logs/server";
    strcat(pathLogs, destino);
    strcat(pathLogs, ".log\0");
    logger = log_create(pathLogs, "log_cliente", true, LOG_LEVEL_INFO);	

	int puerto;
    t_config *config;
    config = iniciar_config();
	
    //config = config_create("../../utils/src/configs/serverPorts.config");
    config = config_create("../utils/src/configs/serverPorts.config");
    if (config == NULL)
    {
        printf("No encontro el archivo Config\n");
        exit(MSG_ERRQUEUE);
    }
    char selPuerto [9] = "PUERTO\0";
    strcat(selPuerto, destino);
    
    puerto = config_get_int_value(config, selPuerto);

    log_info(logger, "Leí el puerto puerto: %d\n", puerto);
	
    config_destroy(config);

	int server_fd = iniciar_socket(puerto);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	//t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
            /*
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
            */
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

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