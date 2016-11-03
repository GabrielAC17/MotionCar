#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "lp.h"


bool GPIOExport(int pino)
{
	char 	buffer[3];
	int 	arquivo;

	arquivo = open("/sys/class/gpio/export", O_WRONLY);
	if( arquivo == -1 )
		return false;
	
	snprintf( buffer, 3, "%d", pino );
	if( write( arquivo, buffer, 3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIOUnexport(int pino)
{
	char 	buffer[3];
	int 	arquivo;

	arquivo = open("/sys/class/gpio/unexport", O_WRONLY);
	if( arquivo == -1 )
		return false;
	
	snprintf( buffer, 3, "%d", pino );
	if( write( arquivo, buffer, 3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIODirection(int pino, int direcao)
{
	char 	caminho[35];
	int	arquivo;

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/direction", pino);
	arquivo = open(caminho, O_WRONLY);
	if( arquivo == -1 )
		return false;

	if( write( arquivo, (direcao == ENTRADA)?"in":"out", (direcao == ENTRADA)?2:3 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIOWrite(int pino, int valor)
{
	char 	caminho[35];
	int	arquivo;

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
	arquivo = open(caminho, O_WRONLY);
	if( arquivo == -1 )
		return false;

	if( write( arquivo, (valor == HIGH)?"1":"0", 1 ) == -1 )
	{
		close(arquivo);
		return false;
	}
	
	close(arquivo);
	return true;
}

bool GPIORead(int pino){
	char 	caminho[35];
	int		arquivo;
	char 	retorno[2];

	snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
	arquivo = open(caminho, O_RDONLY);
	if( arquivo == -1 )
		return false;

	if( read(arquivo,retorno,2) == -1 )
	{
		close(arquivo);
		return false;
	}

	close(arquivo);
	return retorno[0]-'0';
}
