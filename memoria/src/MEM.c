#include <MEM.h>

int iniciarServerMEM(void)
{
    //ojo con este log
    logger = log_create("./src/logs/log.log","Servidor Memoria",true,LOG_LEVEL_INFO);
    //log_info(logger,"Holasss");
    int server_fd = iniciar_servidor();
    //printf("el socket es %d",server_fd);
    //int cliente_fd = esperar_cliente();
    int cliente_fd = esperar_cliente(server_fd);
    
    while (1) {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op)
        {
        case MESSAGE:
            recibir_mensaje(cliente_fd);
            break;
        
        default:
            break;
        }
    }


    log_destroy(logger);
}