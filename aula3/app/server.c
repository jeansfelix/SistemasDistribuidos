// A multithread echo server 

#include "mysocket.h"  
#include <pthread.h>

#define BUFSIZE 100
#define NTHREADS 100

/* Structure of arguments to pass to client thread */
struct TArgs {
  TSocket cliSock;   /* socket descriptor for client */
  int idCliente;
};

pthread_mutex_t lock;

int numClientesAtivos = 0;
int idCliente;

int executarOperacao(int idCliente, TSocket cliSock, char* str, char* ret)
{
  char somar[]    = "somar";
  char subtrair[] = "subtrair";
  char eco[]      = "eco";
  char clientes[] = "clientes";
  
  char opLida[100];
  double valor1, valor2;

  sscanf(str,"%s", opLida);
  
  if (strncmp(opLida, somar, 5) == 0)
  {
    sscanf(str,"%s %lf %lf", opLida, &valor1, &valor2);
    sprintf(ret, "%lf", valor1 + valor2);
  }
  else if (strncmp(opLida, subtrair, 8) == 0) 
  {
    sscanf(str,"%s %lf %lf", opLida, &valor1, &valor2);
    sprintf(ret, "%lf", valor1 - valor2);
  }
  else if (strncmp(opLida, eco, 3) == 0) 
  {
    sscanf(str,"%s %[^\n]", opLida, ret);
  }
  else if (strncmp(opLida, clientes, 8) == 0)
  {
    pthread_mutex_lock(&lock);
    sprintf(ret, "%d", numClientesAtivos);
    pthread_mutex_unlock(&lock);   
  }
  else if (strncmp(opLida, "sair", 4) == 0) 
  {
    if (WriteN(cliSock, opLida, 100) <= 0) ExitWithError("WriteN() failed");
    return 1;
  }
  else
  {
    sprintf(ret, "%s", "Operacao nao permitida.");
  }
  
  return 0;
}

/* Handle client request */
void * HandleRequest(void *args) 
{
  char str[100];
  TSocket cliSock;
  int idCliente;
  char response[100] = {'\0'};

  /* Extract socket file descriptor from argument */
  cliSock = ((struct TArgs *) args) -> cliSock;
  idCliente = ((struct TArgs *) args) -> idCliente;
  
  free(args);  /* deallocate memory for argument */

  for(;;) 
  {
    memset(response,0,sizeof(response));
  
    /* Receive the request */
    if (ReadLine(cliSock, str, 99) < 0) ExitWithError("ReadLine() failed"); 

    if (executarOperacao(idCliente, cliSock, str, response) == 1) break;

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
  int tid = 0;
  fd_set set;  /* file description set */
  int ret, i;
  char str[BUFSIZE];

  if (argc == 1) { ExitWithError("Usage: server <local port>"); }

  /* Create a passive-mode listener endpoint */  
  srvSock = CreateServer(atoi(argv[1]));

  printf("Servidor iniciado!\n");
  /* Run forever */
  for (;;) { 
    /* Initialize the file descriptor set */
    FD_ZERO(&set);
    /* Include stdin into the file descriptor set */
    FD_SET(STDIN_FILENO, &set);
    /* Include srvSock into the file descriptor set */
    FD_SET(srvSock, &set);

    /* Select returns 1 if input available, -1 if error */
    ret = select (FD_SETSIZE, &set, NULL, NULL, NULL);
    if (ret<0) 
    {
       WriteError("select() failed"); 
       break;
    }

    /* Read from stdin */
    if (FD_ISSET(STDIN_FILENO, &set))
    {
      scanf("%99[^\n]%*c", str);
      if (strncmp(str, "TERMINAR_SERVIDOR", 17) == 0) break;
    }

    /* Read from srvSock */
    if (FD_ISSET(srvSock, &set))
    {
      if (tid == NTHREADS)
      { 
        WriteError("number of threads is over"); 
        break; 
      }
      
      /* Spawn off separate thread for each client */
      cliSock = AcceptConnection(srvSock);
      
      pthread_mutex_lock(&lock);
      numClientesAtivos++;
      idCliente++;

      /* Create separate memory for client argument */
      if ((args = (struct TArgs *) malloc(sizeof(struct TArgs))) == NULL) 
      { 
        WriteError("malloc() failed"); 
        break;
      }
      args->cliSock = cliSock;
      args->idCliente = idCliente;
      
      pthread_mutex_unlock(&lock);

      /* Create a new thread to handle the client requests */
      if (pthread_create(&threads[tid++], NULL, HandleRequest, (void *) args))
      { 
        WriteError("pthread_create() failed"); 
        break;
      }
    }
  }
  
  puts("Servidor esperando clientes terminarem!\n");
  /* Wait for all threads to terminate */
  for(i=0; i<tid; i++) 
  {
    pthread_join(threads[i], NULL);
  }
  
  puts("Servidor encerrado!\n");
  
  return 0;
}
