#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS 10
#define Q_SIZE 8
#define SOCKET_ERROR -1
#define MESSAGE "This is my message: I'm so excited!!!"
#define BUFFER_SIZE 100

int queue[Q_SIZE];
int num = 0;
int add = 0;
int rem = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_t workers[NUM_THREADS];

void *boss(void *data);
void *worker(void *data);

int server_create(void *port_num_ptr)
{
  pthread_t boss_thread;
  int e, i;
  e = pthread_create(&boss_thread, NULL, boss, port_num_ptr);
  assert(e == 0);

  for(i = 0; i < NUM_THREADS; i++)
  {
    e = pthread_create(&workers[i], NULL, worker, NULL);
    assert(e == 0);
  }

  e = pthread_join(boss_thread, NULL);
  assert(e == 0);

  for(i = 0; i < NUM_THREADS; i++)
  {
    e = pthread_join(workers[i], NULL);
    assert(e == 0);
  }
  return 0;
}

void *boss(void *data)
{
  int hSocket, hServerSocket; /* handle to socket */
  struct sockaddr_in address; /* Internet socket address struct */
  int nAddressSize = sizeof(struct sockaddr_in);
  int nHostPort = *(int *)data;
  printf("Starting server\n");

  hServerSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(hServerSocket == SOCKET_ERROR)
  {
    printf("Could not make a socket\n");
    return NULL;
  }

  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(nHostPort);
  address.sin_family = AF_INET;

  if(bind(hServerSocket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
  {
    printf("Could not connect to host\n");
    return NULL;
  }
  getsockname(hServerSocket, (struct sockaddr *)&address, (socklen_t *)&nAddressSize);

  if(listen(hServerSocket, 10) == SOCKET_ERROR)
  {
    printf("Could not listen\n");
    return NULL;
  }
  
  printf("Listening...\n");
  while(1)
  {
    hSocket = accept(hServerSocket, (struct sockaddr *)&address, (socklen_t *)&nAddressSize);
    printf("Got a connection\n");
    pthread_mutex_lock(&lock);
    while(num == Q_SIZE)
      pthread_cond_wait(&full, &lock);
    queue[add] = hSocket;
    add = (add + 1) % Q_SIZE;
    num++;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&empty);
    printf("Added socket %d\n", hSocket);
  }
  return NULL;
}

void *worker(void *data)
{
  int hSocket;
  char pBuffer[BUFFER_SIZE];
  while(1)
    {
      pthread_mutex_lock(&lock);
      while(num == 0)
	pthread_cond_wait(&empty, &lock);
      hSocket = queue[rem];
      rem = (rem + 1) % Q_SIZE;
      num--;
      pthread_mutex_unlock(&lock);
      pthread_cond_signal(&full);
      
      /* Process information */
      strcpy(pBuffer, MESSAGE);
      write(hSocket, pBuffer, strlen(pBuffer) + 1);
      printf("Wrote to socket %d\n", hSocket);
      /*read(hSocket, pBuffer, BUFFER_SIZE);
      if(strcmp(pBuffer, MESSAGE) == 0)
	printf("The messages match\n");
      else
	printf("Something was changed\n");
      */
      if(close(hSocket) == SOCKET_ERROR)
      {
	printf("Could not close socket\n");
	return NULL;
      }
      
    }
  return NULL;
}
