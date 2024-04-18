#include <clientToKer.h>

//#define destino "CPU"
//#define destino "MEM"
#define destino "KER"

int clientToKer(void)
{
    /*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

    int conexion;
    char *ip;
    char *puerto;
    // char* valor;

    t_log *logger;
    t_config *config;

    /* ---------------- LOGGING ---------------- */
    logger = iniciar_logger();
    char pathLogs [29] = ".//src/logs/clientTo";
    strcat(pathLogs, destino);
    strcat(pathLogs, ".log\0");
    logger = log_create(pathLogs, "log_cliente", true, LOG_LEVEL_INFO);

    /* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

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
    
    ip = config_get_string_value(config, "IP");
    puerto = config_get_string_value(config, selPuerto);

    log_info(logger, "Leí la ip: %s y puerto: %s\n", ip, puerto);

    config_destroy(config);

    /* ---------------- LEER DE CONSOLA ---------------- */

    leer_consola(logger);
    log_destroy(logger);

    /*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

    // ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

    // Creamos una conexión hacia el servidor
    conexion = crear_conexion(ip, puerto);
    // Enviamos al servidor el valor de CLAVE como mensaje
    // Armamos y enviamos el paquete
    paquete(conexion);
    terminar_programa(conexion, logger, config);
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

void leer_consola(t_log *logger)
{
    char *leido;

    while (1)
    {
        leido = readline("> ");

        if (strcmp(leido, "") == 0)
        {
            break;
        }

        log_info(logger, leido);
        printf("%s/n", leido);

        free(leido);
    }
}

void paquete(int conexion)
{
    // Ahora toca lo divertido!
    char *leido;
    t_paquete *paquete;

    // Leemos y esta vez agregamos las lineas al paquete
    // ¡No te olvides de liberar las líneas y el paquete antes de regresar!
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
    /* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
      con las funciones de las commons y del TP mencionadas en el enunciado */
    free(logger);
    free(config);
}