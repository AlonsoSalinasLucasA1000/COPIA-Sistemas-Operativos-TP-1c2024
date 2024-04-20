#include <IO.h>

int iniciarClienteIO(void)
{
    int conexion;
    char* ip;
    char* puerto;
    char* valor;

    t_log* logger;
    t_config* config;

    logger = log_create("/home/utnso/Documents/logDePrueba.log","PruebaIO",true,LOG_LEVEL_INFO);
    config = config_create("../utils/cliente.config");
    if( config == NULL )
    {
        printf("fail");
    }
    else
    {
        ip = config_get_string_value(config,"IP");
        log_info(logger,ip);
    }

    ip = config_get_string_value(config,"IP");
    puerto = config_get_string_value(config,"PUERTO");
    valor = config_get_string_value(config,"CLAVE");

    conexion = crear_conexion(ip,puerto);

    //LUEGO SEGUIMOS
}