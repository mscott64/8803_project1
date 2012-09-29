#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 10
#define Q_SIZE 8

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
  int i;
  for(i = 0; i < 50; i++)
  {
    pthread_mutex_lock(&lock);
    while(num == Q_SIZE)
      pthread_cond_wait(&full, &lock);
    queue[add] = i + 1;
    add = (add + 1) % Q_SIZE;
    num++;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&empty);
  }
  return NULL;
}

void *worker(void *data)
{
  int d;
  while(1)
    {
      pthread_mutex_lock(&lock);
      while(num == 0)
	pthread_cond_wait(&empty, &lock);
      d = queue[rem];
      rem = (rem + 1) % Q_SIZE;
      num--;
      pthread_mutex_unlock(&lock);
      pthread_cond_signal(&full);
      printf("Thread num: %d\n", d);
    }
  return NULL;
}
