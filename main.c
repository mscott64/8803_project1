#include <ctype.h>
#include <server.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_PORT_NUM 1234

void printUsage(void);

int main(int argc, char *argv[])
{
  int c;
  int port_num = DEFAULT_PORT_NUM;
  while((c = getopt(argc, argv, "p:")) != -1)
  {
    switch(c)
    {
    case 'p':
      port_num = atoi(optarg);
      break;
    case '?':
      if(optopt == 'p')
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
  return server_create(&port_num);
}

void printUsage(void)
{
  printf("Usage: main [-p port_number]\n");
}
