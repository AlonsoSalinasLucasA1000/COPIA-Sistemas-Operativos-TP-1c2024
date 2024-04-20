#include <IO.h>

int iniciarClienteIO(void)
{
    int conexion;
    char* ip;
    char* puerto;
    char* valor;

    t_log* logger;
    t_config* config;

    logger = log_create("./src/logDePrueba.log","PruebaIO",true,LOG_LEVEL_INFO);
    if( logger == NULL )
    {
        printf("No encontre");
    }
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

    enviar_mensaje("/Prueba de otro dia/",conexion);

    log_destroy(logger);
    config_destroy(config);

    //LUEGO SEGUIMOS
}