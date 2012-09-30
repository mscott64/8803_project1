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
#define BUFFER_SIZE 10000
#define NOT_FOUND_ERROR "404 Not Found"
#define BAD_REQUEST "400 Bad Request"
#define INTERNAL_ERROR "500 Internal Error"

int queue[Q_SIZE];
int num = 0;
int add = 0;
int rem = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_t workers[NUM_THREADS];

int reqs = 0;
pthread_mutex_t reqs_lock = PTHREAD_MUTEX_INITIALIZER;
void *boss(void *data);
void *worker(void *data);

void *server_create(void *data)
{
  pthread_t boss_thread;
  int e, i;
  e = pthread_create(&boss_thread, NULL, boss, data);
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
  return NULL;
}

void *boss(void *data)
{
  int hSocket, hServerSocket; /* handle to socket */
  struct sockaddr_in address; /* Internet socket address struct */
  int nAddressSize = sizeof(struct sockaddr_in);
  int nHostPort = *(int *)data;
  //printf("Starting server\n");

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
  
  //printf("Listening...\n");
  while(1)
  {
    hSocket = accept(hServerSocket, (struct sockaddr *)&address, (socklen_t *)&nAddressSize);
    //printf("Got a connection\n");
    pthread_mutex_lock(&lock);
    while(num == Q_SIZE)
      pthread_cond_wait(&full, &lock);
    queue[add] = hSocket;
    add = (add + 1) % Q_SIZE;
    num++;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&empty);
    //printf("Added socket %d\n", hSocket);
  }
  return NULL;
}

void *worker(void *data)
{
  int hSocket;
  char pBuffer[BUFFER_SIZE];
  char method[BUFFER_SIZE];
  char path[BUFFER_SIZE];
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
      //strcpy(pBuffer, MESSAGE);
      //write(hSocket, pBuffer, strlen(pBuffer) + 1);
      //printf("Wrote to socket %d\n", hSocket);
      read(hSocket, pBuffer, BUFFER_SIZE);
      if(sscanf(pBuffer, "%[^ ] %[^ ]", method, path) != 2)
	{
	  //write(hSocket, BAD_REQUEST, strlen(BAD_REQUEST) + 1);
	  printf("Bad request\n");
	  return NULL;
	}

      int curr;
      pthread_mutex_lock(&reqs_lock);
      curr = ++reqs;
      pthread_mutex_unlock(&reqs_lock);

      printf("method: %s path: %s %d\n", method, path, curr);
      if(close(hSocket) == SOCKET_ERROR)
      {
	printf("Could not close socket\n");
	return NULL;
      }
      /*if(strcmp(pBuffer, MESSAGE) == 0)
      {
	int curr;
	pthread_mutex_lock(&reqs_lock);
	curr = ++reqs;
	pthread_mutex_unlock(&reqs_lock);
	printf("The messages match %d\n", curr);
      }
      else
      printf("Something was changed\n");*/
  
    }
  return NULL;
}
