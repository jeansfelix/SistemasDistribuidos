// A multithread echo server 

#include "mysocket.h"
#include <pthread.h>
#include <string.h>

#define NTHREADS 100

/* Structure of arguments to pass to client thread */
struct TArgs {
  TSocket cliSock;   /* socket descriptor for client */
};

double executarOperacao(char* str)
{
  char somar[] = "somar";
  char subtrair[] = "subtrair";
  char opLida[20];
  
  double valor1, valor2;

  sscanf(str,"%s %lf %lf", opLida, &valor1, &valor2);
  
  if (strcmp(opLida, somar) == 0)
  {
    return valor1 + valor2;
  }
  else if (strcmp(opLida, subtrair) == 0) 
  {
    return valor1 - valor2;
  }
  else
  {
    ExitWithError("Operação inválida!!");
  }

  return 0;
}

/* Handle client request */
void * HandleRequest(void *args) 
{
  char str[100];
  TSocket cliSock;
  char response[100] = {'\0'};
  char limpar[100] = {'\0'};

  /* Extract socket file descriptor from argument */
  cliSock = ((struct TArgs *) args) -> cliSock;
  free(args);  /* deallocate memory for argument */

  for(;;) 
  {
    /* Receive the request */
    if (ReadLine(cliSock, str, 99) < 0) ExitWithError("ReadLine() failed"); 

    sprintf(response, "%s", limpar);
    sprintf(response, "%lf", executarOperacao(str));
    
    puts(response);
    
    /* Send the response */
    if (WriteN(cliSock, response, 100) <= 0)
    {
      ExitWithError("WriteN() failed");
    }  
  }
  
  close(cliSock);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
  TSocket srvSock, cliSock;        /* server and client sockets */
  struct TArgs *args;              /* argument structure for thread */
  pthread_t threads[NTHREADS];
  int tid = 0;

  if (argc == 1) { ExitWithError("Usage: server <local port>"); }

  /* Create a passive-mode listener endpoint */  
  srvSock = CreateServer(atoi(argv[1]));

  /* Run forever */
  for (;;) { 
    if (tid == NTHREADS) ExitWithError("number of threads is over");

    /* Spawn off separate thread for each client */
    cliSock = AcceptConnection(srvSock);

    /* Create separate memory for client argument */
    if ((args = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) ExitWithError("malloc() failed");
    
    args->cliSock = cliSock;

    /* Create a new thread to handle the client requests */
    if (pthread_create(&threads[tid++], NULL, HandleRequest, (void *) args)) ExitWithError("pthread_create() failed");
    
    /* NOT REACHED */
  }
}
