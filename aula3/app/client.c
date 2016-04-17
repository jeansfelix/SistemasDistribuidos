// echo client 
#include "mysocket.h"  

int main(int argc, char *argv[]) {
  TSocket sock;
  char *servIP;                /* server IP */
  unsigned short servPort;     /* server port */
  char str[100];
  int n;

  if (argc != 3) 
  {
    ExitWithError("Usage: client <remote server IP> <remote server Port>");    
  }

  servIP = argv[1];
  servPort = atoi(argv[2]);

  /* Create a connection */
  sock = ConnectToServer(servIP, servPort);
  
  puts("Conexão iniciada!");

  for(;;) 
  {
    /* Write msg */
    scanf(" %[^\n]",str);
    n = strlen(str);
    str[n] = '\n';
    
    if (WriteN(sock, str, ++n) <= 0) ExitWithError("WriteN() failed");

    /* Receive the response */
    if (ReadN(sock, str, 100) < 0) ExitWithError("ReadLine() failed");

    if (strcmp(str, "sair") == 0) break;
    
    str[strlen(str)] = '\n';
    
    printf("%s",str);
  }

  puts("Conexão encerrada!");

  close(sock);
  return 0;
}
