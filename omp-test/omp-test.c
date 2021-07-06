#include <stdio.h>
#include <omp.h>

int main(int argc, char ** argv)
{
#pragma omp parallel for
    for(unsigned int i = 0; i < 10; ++i)
    {
        unsigned int thread = omp_get_thread_num();
        printf("Thread %d is doing iteration %d.\n", thread, i);
    }
    
    return 0;
}
