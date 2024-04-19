#ifndef GENERALS_H_
#define GENERALS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;



#endif /* GENERALS_H_ */