#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS 5
#define BUFFER_SIZE 10000
#define HOST_NAME_SIZE 100
#define SOCKET_ERROR -1

void *load(void *data);

void* client_create(void *data)
{
  pthread_t load_threads[NUM_THREADS];
  int i, e;
  for(i = 0; i < NUM_THREADS; i++)
  {
    e = pthread_create(&load_threads[i], NULL, load, data);
    assert(e == 0);
  }

  for(i = 0; i < NUM_THREADS; i++)
  {
    e = pthread_join(load_threads[i], NULL);
    assert(e == 0);
  }
  return NULL;
}

void *load(void *data)
{
  int i;
  int hSocket; /* handle to socket */
  struct hostent *pHostInfo; /* holds info about machine */
  struct sockaddr_in address; /* Internet socket address struct */
  long nHostAddress;
  char strHostName[HOST_NAME_SIZE];
  //char output[BUFFER_SIZE];
  int nHostPort, numRequests;
  char *request;
  
  int *info_ptr = (int *)data;
  strcpy(strHostName, *(char **)info_ptr++);
  nHostPort = *(int *)info_ptr++;
  numRequests = *(int *)info_ptr++;
  request = *(char **)info_ptr;
  //printf("Connecting to %s on port %d\n", strHostName, nHostPort);
 
  //printf("Get host information\n");
  pHostInfo = gethostbyname(strHostName);
  memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);
  
  address.sin_addr.s_addr = nHostAddress;
  address.sin_port = htons(nHostPort);
  address.sin_family = AF_INET;
  for(i = 0; i < numRequests; i++)
  {
    
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(hSocket == SOCKET_ERROR)
    {
      printf("Could not make socket\n");
      return NULL;
    }
    
    if(connect(hSocket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
      printf("Could not connect to host\n");
      return NULL;
    }
    printf("%s\n", request);
    write(hSocket, request, strlen(request) + 1);
    //read(hSocket, output, BUFFER_SIZE);
    //output[BUFFER_SIZE - 1] = 0;

    if(close(hSocket) == SOCKET_ERROR)
    {
      printf("Could not close socket\n");
      return 0;
    }
  }
  return NULL;
}
