#include <clientsIO.h>

//#define destino "CPU"
#define destino "MEM"
//#define destino "KER"

int clientToMEM(void)
{
    /*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

    int conexion;
    char* ip = malloc(sizeof(char) * 16);
    
    // char* valor;

    t_log *logger;
    t_config *config;

    /* ---------------- LOGGING ---------------- */
    char pathLogs [29] = ".//src/logs/clientTo";
    strcat(pathLogs, destino);
    strcat(pathLogs, ".log\0");
    logger = log_create(pathLogs, "log_cliente", true, LOG_LEVEL_INFO);

    /* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
    //config = config_create("../../utils/src/configs/serverPorts.config");
    config = config_create("../utils/src/configs/serverPorts.config");
    
    if (config == NULL)
    {
        printf("No encontro el archivo Config\n");
        exit(MSG_ERRQUEUE);
    }
    char selPuerto [9] = "PUERTO\0";
    strcat(selPuerto, destino);
    
    strcpy(ip, config_get_string_value(config, "IP"));
    int puerto = config_get_int_value(config, selPuerto);

    log_info(logger, "Leí la ip: %s y puerto: %d\n", ip, puerto);

    config_destroy(config);

    /* ---------------- LEER DE CONSOLA ---------------- */

    //leer_consola(logger);
    log_destroy(logger);

    /*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

    // ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

    // Creamos una conexión hacia el servidor
    conexion = crear_conexion(ip, puerto);
    enviar_mensaje("123456789", conexion);
    // Enviamos al servidor el valor de CLAVE como mensaje
    // Armamos y enviamos el paquete
    paquete(conexion);
    terminar_programa(conexion, logger, config);
    return conexion;
}