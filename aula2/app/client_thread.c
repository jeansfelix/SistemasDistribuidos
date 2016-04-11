// echo client 
#include "mysocket.h"  

int main(int argc, char *argv[]) {
  TSocket sock;
  char *servIP;                /* server IP */
  unsigned short servPort;     /* server port */
  char str[100];
  char opEscrita[100];
  int n;

  if (argc != 3) 
  {
    ExitWithError("Usage: client <remote server IP> <remote server Port>");    
  }

  servIP = argv[1];
  servPort = atoi(argv[2]);

  /* Create a connection */
  sock = ConnectToServer(servIP, servPort);

  for(;;) 
  {
    /* Write msg */
    scanf(" %[^\n]",str);
    n = strlen(str);
    str[n] = '\n';
    
    if (WriteN(sock, str, ++n) <= 0) ExitWithError("WriteN() failed");

    if (strcmp(opEscrita, "sair") == 0) break;

    /* Receive the response */
    if (ReadN(sock, str, 100) < 0) ExitWithError("ReadLine() failed");
    
    str[strlen(str)] = '\n';
    
    printf("%s",str);
  }

  close(sock);
  return 0;
}
