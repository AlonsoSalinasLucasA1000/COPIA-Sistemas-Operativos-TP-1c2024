#include "serverMemoria.h"
#include <commons/config.h>

//#define destino "CPU"
#define destino "MEM"
//#define destino "KER"

int iniciar_servidor() {
	
	char* lecturaConsola;
	t_log *logger;

	char pathLogs [27] = ".//src/logs/server";//CAMBIAR
    strcat(pathLogs, destino);
    strcat(pathLogs, ".log\0");
    logger = log_create(pathLogs, "log_cliente", true, LOG_LEVEL_INFO);	

	int puerto;
    t_config *config;

    //config = config_create("../../utils/src/configs/serverPorts.config");
    config = config_create("../utils/src/configs/serverPorts.config");
    if (config == NULL)
    {
        printf("No encontro el archivo Config\n");
        exit(MSG_ERRQUEUE);
    }

	//CAMBIAR
    char selPuerto [9] = "PUERTO\0";
    strcat(selPuerto, destino);
    
    puerto = config_get_int_value(config, selPuerto);

    log_info(logger, "Leí el puerto puerto: %d\n", puerto);
	
    config_destroy(config);

	do{
		int server_fd = iniciar_socket(puerto);
		log_info(logger, "Servidor listo para recibir al cliente");
		int cliente_fd = esperar_cliente(server_fd);
		printf("\nsocket server == %d\n", server_fd);
		printf("\nEstoy conectado con el socket %d\n", cliente_fd);

		//t_list* lista;
		bool k = true;
		while (k) {
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
				log_error(logger, "el cliente se desconecto.");
				k = false;
				break;
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
		fflush(stdin);
		lecturaConsola = readline("\nIngrese una `q` para finalizar el servidor:\n>");
		printf("Lectura consola == %s\n",lecturaConsola);
	} while (strcmp(lecturaConsola, "exit\n")!=0);
	
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}