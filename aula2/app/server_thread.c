// A multithread echo server 

#include "mysocket.h"
#include <pthread.h>
#include <string.h>

#define NTHREADS 100

/* Structure of arguments to pass to client thread */
struct TArgs {
  TSocket cliSock;   /* socket descriptor for client */
};

pthread_mutex_t lock;

int numClientesAtivos = 0;

void executarOperacao(char* str, char* ret)
{
  char somar[] = "somar";
  char subtrair[] = "subtrair";
  char eco[] = "eco";
  char clientes[] = "clientes";
  
  char opLida[100];
  
  double valor1, valor2;

  sscanf(str,"%s", opLida);
  
  if (strcmp(opLida, somar) == 0)
  {
    sscanf(str,"%s %lf %lf", opLida, &valor1, &valor2);
    sprintf(ret, "%lf", valor1 + valor2);
  }
  else if (strcmp(opLida, subtrair) == 0) 
  {
    sscanf(str,"%s %lf %lf", opLida, &valor1, &valor2);
    sprintf(ret, "%lf", valor1 - valor2);
  }
  else if (strcmp(opLida, eco) == 0) 
  {
    sscanf(str,"%s %[^\n]", opLida, ret);
  }
  else if (strcmp(opLida, clientes) == 0)
  {
    pthread_mutex_lock(&lock);
    sprintf(ret, "%d", numClientesAtivos);
    pthread_mutex_unlock(&lock);   
  }
  else
  {
    sprintf(ret, "%s", "Operacao nao permitida.");
  }
}

/* Handle client request */
void * HandleRequest(void *args) 
{
  char str[100];
  TSocket cliSock;
  char response[100] = {'\0'};
  char opLida[100];

  /* Extract socket file descriptor from argument */
  cliSock = ((struct TArgs *) args) -> cliSock;
  free(args);  /* deallocate memory for argument */

  pthread_mutex_lock(&lock);
  numClientesAtivos++;
  pthread_mutex_unlock(&lock);

  for(;;) 
  {
    memset(response,0,sizeof(response));
  
    /* Receive the request */
    if (ReadLine(cliSock, str, 99) < 0) ExitWithError("ReadLine() failed"); 


    sscanf(str,"%s", opLida);
    if (strcmp(opLida, "sair") == 0) break;
    
    executarOperacao(str, response);

    puts(response);
    
    /* Send the response */
    if (WriteN(cliSock, response, 100) <= 0)
    {
      ExitWithError("WriteN() failed");
    }  
  }
  
  close(cliSock);
  
  pthread_mutex_lock(&lock);
  numClientesAtivos--;
  pthread_mutex_unlock(&lock);
  
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
  TSocket srvSock, cliSock;        /* server and client sockets */
  struct TArgs *args;              /* argument structure for thread */
  pthread_t threads[NTHREADS];
  int i, tid = 0;

  if (argc == 1) { ExitWithError("Usage: server <local port>"); }

  /* Create a passive-mode listener endpoint */  
  srvSock = CreateServer(atoi(argv[1]));

  /* Run forever */
  for (;;) 
  { 
    if (tid == NTHREADS) ExitWithError("number of threads is over");

    /* Spawn off separate thread for each client */
    cliSock = AcceptConnection(srvSock);

    /* Create separate memory for client argument */
    if ((args = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) ExitWithError("malloc() failed");
    
    args->cliSock = cliSock;

    /* Create a new thread to handle the client requests */
    if (pthread_create(&threads[tid++], NULL, HandleRequest, (void *) args)) ExitWithError("pthread_create() failed");

  }
  
  printf("Server will wait for the active threads and terminate!\n");
  /* Wait for all threads to terminate */
  for(i=0; i<tid; i++)
  {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
