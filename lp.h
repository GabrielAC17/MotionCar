#ifndef lp_h
#define lp_h

#define ENTRADA 1
#define SAIDA	0
#define INPUT 	1
#define OUTPUT	0
#define HIGH	1
#define LOW	0

bool GPIOExport(int pino);
bool GPIODirection(int pino, int direcao);
bool GPIOWrite(int pino, int valor);
bool GPIORead(int pino);
bool GPIOUnexport(int pino);

#endif
