#include <assert.h>
#include <client.h>
#include <ctype.h>
#include <pthread.h>
#include <server.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_PORT_NUM 1234

void printUsage(void);

int main(int argc, char *argv[])
{
  pthread_t client_thread, server_thread;
  int c;
  int port_num = DEFAULT_PORT_NUM;
  char *hostname = "ubuntu";
  opterr = 0;
  while((c = getopt(argc, argv, "h:p:")) != -1)
  {
    switch(c)
    {
    case 'h':
      hostname = optarg;
      break;
    case 'p':
      port_num = atoi(optarg);
      break;
    case '?':
      if(optopt == 'p'|| optopt == 'h')
	printf("Option -%c requires an argument\n", optopt);
      else
	printf("Unknown option -%c\n", optopt);
    default:
      printUsage();
      return 0;
    }
  }
  if(port_num < 1)
  {
    printf("Invalid port number %d\n", port_num);
    return 0;
  }
  
  int e;
  e = pthread_create(&server_thread, NULL, server_create, &port_num);
  assert(e == 0);

  e = pthread_create(&client_thread, NULL, client_create, NULL);
  assert(e == 0);
  
  pthread_join(client_thread, NULL);
  return 0;
}

void printUsage(void)
{
  printf("Usage: main [-p port_number] [-h hostname]\n");
}
