#include "utils.h"


int
main(int a, char** v)
{
  int core = the_cores[1];

  if (a > 1)
    {
      core = the_cores[atoi(v[1])];
    }

  printf("running on core %d\n", core);
  set_cpu(core);

  volatile long long unsigned int i, j, k = 0;
  while (1)
    {
      for (i = 2; i < 2e32; i++)
	{
	  for (j = 2; j < i; j++)
	    {
	      if (i % j == 0)
		{
		  k--;
		  break;
		}
	    }
	  k++;
	}

    }

}
