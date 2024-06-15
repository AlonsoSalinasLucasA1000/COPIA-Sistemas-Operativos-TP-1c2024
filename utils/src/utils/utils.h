#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include <pthread.h>
#include<readline/readline.h>
#include<commons/collections/queue.h>
#include <semaphore.h>
#include<commons/collections/list.h> 


typedef enum
{
	MENSAJE,
	PAQUETE,
	PROCESO,
	PROCESOFIN,
	MARCO,
	NUMERO,
	GOKU
}op_code;

/*typedef struct
{
	int size;
	void* stream;
} t_buffer;
*/
typedef struct
{
	uint32_t size; // Tamaño del payload
	uint32_t offset; // Desplazamiento dentro del payload
	void* stream; // Payload
} t_newBuffer;

typedef struct 
{
	op_code codigo_operacion;//paquete
	t_newBuffer* buffer;//buffer serializado
} t_newPaquete;//paquete que queremos enviar

/*typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;*/

typedef enum{
	NEW,
	READY,
	BLOCKED,
	EXEC, //pasa a ejecutar cuando sale entra a la cola de ready
	EXIT
} estado_proceso;

typedef struct
{
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
	uint32_t SI;
	uint32_t DI;
} RegistrosCPU;

typedef struct
{
	uint32_t PID;
	uint32_t PC;
	uint32_t quantum; //es para VRR
	RegistrosCPU registro;//descomente para implementarlo
	estado_proceso estado;
	uint32_t path_length;
	char* path;
} PCB;

typedef struct
{
	uint32_t PID;
	uint32_t path_length;
	char* path;
} ProcesoMemoria;

typedef struct
{
	uint32_t PID;
	int pagina;
	int marco; //
	//si se necesita se puede agrgar campos extras
} TLB;




//funciones crear conexion
int crear_conexion(char* ip, char* puerto,char* nameServ);
int iniciar_servidor(char* puerto, t_log* un_log, char* msj_server);
int esperar_cliente(int socket_servidor, t_log* un_log, char* msj);
int recibir_operacion(int socket_cliente);

//funciones de paquetes, buffers y envios de mensajes
//void crear_buffer(t_paquete* paquete);
void crear_newBuffer(t_newBuffer* paquete);

void crearBufferProcesoMemoria(t_newBuffer* buffer, ProcesoMemoria* proceso);

void rellenarPaqueteConNewBuffer(t_newPaquete* paquete, t_newBuffer* buffer);

void enviarPaqueteConNewBuffer(t_newPaquete* paquete, int socket);

ProcesoMemoria* deserializar_proceso_memoria(t_newBuffer* buffer);

void* recibir_buffer(int* size, int socket_cliente);

void* serializar_paquete(t_newPaquete* paquete, int bytes);

//t_paquete *crear_paquete(void);
void paquete(int conexion,char* lo_que_se_desea_enviar);

//void agregar_a_paquete(t_paquete* paquete, void* valor, size_t tamanio);

void enviar_mensaje(char* mensaje, int socket_cliente);

t_list* recibir_paquete(int socket_cliente);
void enviar_mensaje_cpu_memoria(char* mensaje, int socket_cliente);

void eliminar_paquete(t_newPaquete* paquete);

//funciones propias para la serializacion
void enviarPCB (PCB* proceso, int socket_servidor, op_code codigo);

//handshake y liberar conexiones
void handshakeClient(int fd_servidor, int32_t handshake);
void handshakeServer(int fd_client);
void liberar_conexion(int socket);
void liberar_logger(t_log* logger);
void liberar_config(t_config* config);

#endif /* UTILS_H_ */