#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
struct block
{
  unsigned long tag;
  struct block * next;
};
struct Queue
{
  struct block * front;
  struct block * end;
  unsigned long size;
};
struct counter
{
  unsigned long read;
  unsigned long write;
  unsigned long hit;
  unsigned long miss;
};
int powerOfTwo(int x) // Returns 0 if false, 1 if true
{
  if (x <= 0)
    {
      return 0;
    }
  while (x != 1)
    {
      if (x % 2 != 0)
	{
	  return 0;
	}
      x /= 2;
    }
  return 1;
}
unsigned int logarithm(unsigned int n)
{
  return (n > 1) ? 1 + logarithm(n/2) : 0;
}
struct block * makeBlock(unsigned long t)
{
  struct block * b = malloc(sizeof(struct block));
  b -> tag = t;
  b -> next = NULL;
  return b;
}
struct Queue * makeQueue()
{
  struct Queue * q = malloc(sizeof(struct Queue));
  q -> size = 0;
  q -> front = NULL;
  q -> end = NULL;
  return q;
}
int searchBlocks(struct Queue ** c, unsigned long i, unsigned long t)
{
  struct block * pointer = c[i]-> front;
  while (pointer != NULL)
    {
      if (pointer -> tag == t)
	{
	  return 1; //found in cache (hit)
	}
      pointer = pointer->next;
    }
  return 0; // not found in cache (miss)
}
void moveToBack(struct Queue ** c, unsigned long i, unsigned long t)
{
  struct block * pointer = c[i] -> front;
  while (pointer -> tag != t && pointer -> next -> tag != t)
    {
      pointer = pointer -> next;
    }
  if (pointer -> tag == t)
    {
      if (pointer != c[i] -> end)
	{
	  c[i] -> front = c[i] -> front -> next;
	  c[i] -> end -> next = pointer;
	  pointer -> next = NULL;
	  c[i] -> end = pointer;
	}
    }
  else if (pointer -> next -> tag == t)
    {
      if (pointer->next != c[i] -> end)
	{
	  struct block * temp = pointer -> next;
	  pointer -> next = pointer ->next -> next;
	  c[i] -> end -> next = temp;
	  temp -> next = NULL;
	  c[i] -> end = temp;
	}
    }

}
void initializeCache(struct Queue ** c, int s)
{
  int i;
  for (i = 0; i < s; i++)
    {
      c[i] = makeQueue();
    }
}
struct counter * makeCount()
{
  struct counter * count = malloc(sizeof(struct counter));
  count -> read = 0;
  count -> write = 0;
  count -> hit = 0;
  count -> miss = 0;
  return count;
}
void addBlock(struct Queue * q, struct block * b, unsigned long num)
{
  if (q -> size == 0)
		{
		  q -> front = b;
		  q -> end = b;
		  q -> size = q ->size + 1;
		}
	      else if (q -> size < num)
		{
		  q -> end -> next = b;
		  q -> end = b;
		  q -> size = q -> size + 1;
		}
	      else
		{
		  if (num == 1)
		    {
		      free(q -> front);
    
		      q -> front = b;
		      q -> end = b;
		    }
		  else
		    {
		      struct block * temp = q -> front;
		      q -> front = q -> front -> next;
		      free(temp);
		      q -> end -> next = b;
		      q -> end = b;
		    }
		}
}
void read(struct Queue ** c, unsigned long setIndex, unsigned long tag, struct counter * counters, unsigned long numberOfBlocks, char * policy)
{
  int search = searchBlocks(c, setIndex, tag);
  if (search == 1)
    {
      counters -> hit = counters -> hit + 1;
      if (strcmp(policy, "lru") == 0)
	{
	  moveToBack(c, setIndex, tag);
	}
    }
  else
    {
      counters -> miss = counters -> miss + 1;
      counters -> read = counters -> read + 1;
      addBlock(c[setIndex], makeBlock(tag), numberOfBlocks);
    }
} 
void readPrefetch(struct Queue ** c, unsigned long setIndex, unsigned long tag, struct counter * counters, unsigned long numberOfBlocks, unsigned long address, unsigned long p, unsigned long blockSize, unsigned long numberOfSets, char * policy)
{
  int search = searchBlocks(c, setIndex, tag);
  if (search == 1)
    {
      counters -> hit = counters -> hit + 1;
      if (strcmp(policy, "lru") == 0)
	{
	  moveToBack(c, setIndex, tag);
	}
    }
  else
    {
      counters -> miss = counters -> miss + 1;
      counters -> read = counters -> read + 1;
      addBlock(c[setIndex], makeBlock(tag), numberOfBlocks);
      int i;
      for (i = 0; i < p; i++)
	{
	  address += blockSize;
	  unsigned long pTag = address/(blockSize*numberOfSets);
	  unsigned long pSetIndex = (address/blockSize)%numberOfSets;
	  int pSearch = searchBlocks(c, pSetIndex, pTag);
	    {
	      if (pSearch == 0)
		{
		  counters -> read = counters -> read + 1;
		  addBlock(c[pSetIndex], makeBlock(pTag), numberOfBlocks);
		}
	    }
	}
    }
} 
void write(struct Queue ** c, unsigned long setIndex, unsigned long tag, struct counter * counters, unsigned long numberOfBlocks, char * policy)
{
  int search = searchBlocks(c, setIndex, tag);
  if (search == 1)
    {
      counters -> hit = counters -> hit + 1;
      counters -> write = counters -> write + 1;
      if (strcmp(policy, "lru") == 0)
	{
	  moveToBack(c, setIndex, tag);
	}
    }
  else
    {
      counters -> miss = counters -> miss + 1;
      counters -> read = counters -> read + 1;
      counters -> write = counters -> write + 1;
      addBlock(c[setIndex], makeBlock(tag), numberOfBlocks);
    }
}
void writePrefetch(struct Queue ** c, unsigned long setIndex, unsigned long tag, struct counter * counters, unsigned long numberOfBlocks, unsigned long address, unsigned long p, unsigned long blockSize, unsigned long numberOfSets, char * policy)
{
  int search = searchBlocks(c, setIndex, tag);
  if (search == 1)
    {
      counters -> hit = counters -> hit + 1;
      counters -> write = counters -> write + 1;
      if (strcmp(policy, "lru") == 0)
	{
	  moveToBack(c, setIndex, tag);
	}
    }
  else
    {
      counters -> miss = counters -> miss + 1;
      counters -> read = counters -> read + 1;
      counters -> write = counters -> write + 1;
      addBlock(c[setIndex], makeBlock(tag), numberOfBlocks);
      int i;
      for (i = 0; i < p; i++)
	{
	  address += blockSize;
	  unsigned long pTag = address/(blockSize*numberOfSets);
	  unsigned long pSetIndex = (address/blockSize)%numberOfSets;
	  int pSearch = searchBlocks(c, pSetIndex, pTag);
	    {
	      if (pSearch == 0)
		{
		  counters -> read = counters -> read + 1;
		  addBlock(c[pSetIndex], makeBlock(pTag), numberOfBlocks);
		}
	    }
	}
    }
} 
void freeCache(struct Queue ** c, int s)
{
  int i;
  for (i = 0; i < s; i++)
    {
      struct block * previous = NULL;
      struct block * pointer = c[i]->front;
      while (pointer != NULL)
	{
	  previous = pointer;
	  pointer = pointer->next;
	  free(previous);
	}
      free(c[i]);
    }
  free(c);
}
int main(int argc, char ** argv)
{
  if (argc != 7)
    {
      printf("error\n");
      return 0;
    }
  int cacheSize = atoi(argv[1]);
  if (powerOfTwo(cacheSize) == 0)
    {
      printf("error\n");
      return 0;
    }
  int blockSize = atoi(argv[2]);
  if (powerOfTwo(blockSize) == 0)
    {
      printf("error\n");
      return 0;
    }
  char * cachePolicy = argv[3];
  if (strcmp(cachePolicy, "fifo") != 0 && strcmp(cachePolicy, "lru") != 0)
    {
      printf("error\n");
      return 0;
    }
  char * associativity = argv[4];
  int aNum = -1;
  if (strlen(associativity) < 5)
    {
      printf("error\n");
      return 0;
    }
  else if (strlen(associativity) == 5)
    {
      if (strcmp(associativity, "assoc") != 0)
	{
	  printf("error\n");
	  return 0;
	}
    }
  else if (strlen(associativity) == 6)
    {
      if (strcmp(associativity, "direct") != 0)
	{
	  printf("error\n");
	  return 0;
	}
    }
  else
    {
      char assoc[6];
      char assocNum[strlen(associativity)-6];
      int i;
      for (i = 0; i < 6; i++)
	{
	  assoc[i] = associativity[i];
	}
      for (i = 6; i < strlen(associativity); i++)
	{
	  assocNum[i-6] = associativity[i];
	}
      aNum = atoi(assocNum);
      char * temp = "assoc:";
      for (i = 0; i < 6; i++)
	{
	  if (temp[i] != assoc[i])
	    {
	      printf("error\n");
	      return 0;
	    }
	}
      if (powerOfTwo(aNum) == 0)
	{
	  printf("error\n");
	  return 0;
	}
    }
  int prefetchSize = atoi(argv[5]);
  if (prefetchSize < 0)
    {
      printf("error\n");
      return 0;
    }
  FILE * f = fopen(argv[6], "r");
  if (f == NULL)
  {
    printf("error\n");
    return 0;
  }
  unsigned long numberOfSets;
  unsigned long numberOfBlocks;
  if (strcmp(associativity, "direct")==0)
    {
      numberOfSets = cacheSize/blockSize;
      numberOfBlocks = 1;
    }
  else if (strcmp(associativity, "assoc")==0)
    {
      numberOfSets = 1;
      numberOfBlocks = cacheSize/blockSize;
    }
  else
    {
      numberOfSets = cacheSize/aNum/blockSize;
      numberOfBlocks = aNum;
    }
  if (numberOfSets == 0 || numberOfBlocks == 0)
    {
      printf("error\n");
      return 0;
    }
  struct Queue ** cache = malloc(numberOfSets * sizeof(struct Queue));
  initializeCache(cache, numberOfSets);
  struct counter * counters = makeCount();
  char operation;
  unsigned long address;
  int r = fscanf(f, "%c %lx\n", &operation, &address);
  while (r == 2)
    {
      unsigned long tag = address/(blockSize*numberOfSets);
      unsigned long setIndex = (address/blockSize)%numberOfSets;
      if (operation == 'R')
	{
	  read(cache, setIndex, tag, counters, numberOfBlocks, cachePolicy );
	}
      else if (operation == 'W')
	{
	  write(cache, setIndex, tag, counters, numberOfBlocks, cachePolicy);
	}

      r = fscanf(f, "%c %lx\n", &operation, &address);
    }
  printf("no-prefetch\n");
  printf("Memory reads: %lu\n", counters->read);
  printf("Memory writes: %lu\n", counters->write);
  printf("Cache hits: %lu\n", counters->hit);
  printf("Cache misses: %lu\n", counters->miss);
  freeCache(cache, numberOfSets);
  free(counters);


  


  // PREFETCHING PHASE
  FILE * g = fopen(argv[6], "r");
  struct Queue ** cache2 = malloc(numberOfSets * sizeof(struct Queue));
  initializeCache(cache2, numberOfSets);
  struct counter * counters2 = makeCount();
  r = fscanf(g, "%c %lx\n", &operation, &address);
  while (r == 2)
    {
	  unsigned long tag = address/(blockSize*numberOfSets);
	  unsigned long setIndex = (address/blockSize)%numberOfSets;
	  if (operation == 'R')
	    {
	      readPrefetch(cache2, setIndex, tag, counters2, numberOfBlocks, address, prefetchSize, blockSize, numberOfSets, cachePolicy);
	    }
	  else if (operation == 'W')
	    {
	      writePrefetch(cache2, setIndex, tag, counters2, numberOfBlocks, address, prefetchSize, blockSize, numberOfSets, cachePolicy);
	    }
       
      r = fscanf(g, "%c %lx\n", &operation, &address);
    }
  printf("with-prefetch\n");
  printf("Memory reads: %lu\n", counters2->read);
  printf("Memory writes: %lu\n", counters2->write);
  printf("Cache hits: %lu\n", counters2->hit);
  printf("Cache misses: %lu\n", counters2->miss);
  freeCache(cache2, numberOfSets);
  free(counters2);
  return 0;
}
