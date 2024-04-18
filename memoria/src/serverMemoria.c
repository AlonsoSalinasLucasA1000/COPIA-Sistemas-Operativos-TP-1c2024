#include "headerServerMemoria.h"

int prenderServerMR(void) {
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidorMR();
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_clienteMR(server_fd);

	//t_list* lista;
   // int i = 0;
	while (1) {
       // i++;
        // printf("entre al WHILE %d veces",i);
		int cod_op = recibir_operacionMR(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensajeMR(cliente_fd);
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

void iteratorMR(char* value) {
	log_info(logger,"%s", value);
}