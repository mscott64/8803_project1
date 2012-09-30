#include <assert.h>
#include <constants.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <server.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int queue[Q_SIZE];
int num = 0;
int add = 0;
int rem = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_t workers[NUM_THREADS_SERVER];

int reqs = 0;
pthread_mutex_t reqs_lock = PTHREAD_MUTEX_INITIALIZER;
void *boss(void *data);
void *worker(void *data);
void *send_error(int hSocket, char *error_msg);

void *server_create(void *data)
{
  pthread_t boss_thread;
  int e, i;
  e = pthread_create(&boss_thread, NULL, boss, data);
  assert(e == 0);

  for(i = 0; i < NUM_THREADS_SERVER; i++)
  {
    e = pthread_create(&workers[i], NULL, worker, NULL);
    assert(e == 0);
  }

  e = pthread_join(boss_thread, NULL);
  assert(e == 0);

  for(i = 0; i < NUM_THREADS_SERVER; i++)
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
  
  while(1)
  {
    hSocket = accept(hServerSocket, (struct sockaddr *)&address, (socklen_t *)&nAddressSize);
    pthread_mutex_lock(&lock);
    while(num == Q_SIZE)
      pthread_cond_wait(&full, &lock);
    queue[add] = hSocket;
    add = (add + 1) % Q_SIZE;
    num++;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&empty);
    return NULL;
  }
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
      read(hSocket, pBuffer, BUFFER_SIZE);
      if(sscanf(pBuffer, "%[^ ] %[^ ]", method, path) != 2)
	return send_error(hSocket, BAD_REQUEST);
      
      if(strcasecmp(method, "get") != 0)
	return send_error(hSocket, NOT_IMPLEMENTED);

      int len = strlen(path);
      if(path[0] == '/' || strcmp(path, "..") == 0 || strncmp(path, "../", 3) == 0 || strstr(path, "/../") != NULL || strcmp(&(path[len-3]), "/..") == 0)
	return send_error(hSocket, BAD_REQUEST);
      
      FILE *f = fopen(path, "r");
      if(f)
      {
	while(fgets(pBuffer, BUFFER_SIZE, f) != NULL)
	  write(hSocket, pBuffer, BUFFER_SIZE);
	pBuffer[0] = '\0';
	write(hSocket, pBuffer, BUFFER_SIZE);
      }
      else
      {
	return send_error(hSocket, NOT_FOUND);
      }      
      
      if(close(hSocket) == SOCKET_ERROR)
      {
	printf("Could not close socket\n");
	return NULL;
      }
    }
  return NULL;
}

void *send_error(int hSocket, char *error_msg)
{
  char pBuffer[BUFFER_SIZE];
  //write(hSocket, error_msg, strlen(error_msg) + 1);
  printf("%s\n", error_msg);
  pBuffer[0] = '\0';
  write(hSocket, pBuffer, BUFFER_SIZE);
  if(close(hSocket) == SOCKET_ERROR)
    printf("Could not close socket\n");
  
  return NULL;
}
