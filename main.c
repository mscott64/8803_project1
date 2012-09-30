#include <assert.h>
#include <client.h>
#include <constants.h>
#include <ctype.h>
#include <pthread.h>
#include <server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct client_data {
  char *hostname;
  int port_num;
  int num_requests;
  char *request;
};

void printUsage(void);

int main(int argc, char *argv[])
{
  pthread_t client_thread, server_thread;
  int c;
  int port_num = DEFAULT_PORT_NUM;
  char *hostname = DEFAULT_MACHINE;
  int requests = DEFAULT_REQUESTS;
  char *filename = DEFAULT_FILENAME;
  opterr = 0;
  while((c = getopt(argc, argv, "h:p:r:f:")) != -1)
  {
    switch(c)
    {
    case 'f':
      filename = optarg;
      break;
    case 'h':
      hostname = optarg;
      break;
    case 'p':
      port_num = atoi(optarg);
      break;
    case 'r':
      requests = atoi(optarg);
      break;
    case '?':
      if(optopt == 'p'|| optopt == 'h' || optopt == 'r' || optopt == 'f')
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
  if(requests < 1)
  {
    printf("Invalid requests number %d\n", requests);
    return 0;
  }
  
  int e;
  e = pthread_create(&server_thread, NULL, server_create, &port_num);
  assert(e == 0);

  struct client_data data;
  char request[strlen(filename) + GET_LEN + 1];
  memcpy(request, GET, GET_LEN);
  memcpy(request + GET_LEN, filename, strlen(filename) + 1);
  data.hostname = hostname;
  data.port_num = port_num;
  data.num_requests = requests;
  data.request = request;
  e = pthread_create(&client_thread, NULL, client_create, &data);
  assert(e == 0);
  
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);
  return 0;
}

void printUsage(void)
{
  printf("Usage: main\n");
  printf("Options:\n[-p port_number]\n[-h hostname]\n[-r num requests]\n[-f filename]\n");
}
