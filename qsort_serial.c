//#include <mpi.h>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <set>
#include <cstring>
using namespace std;
int BlockSize;
int N = 10000;
int *A;
static void print_numbers(char const * const filename,uint32_t const * const numbers,uint32_t const nnumbers)
{
    FILE * fout;
    
    /* open file */
    if((fout = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "error opening '%s'\n", filename);
        abort();
    }
    
    /* write the header */
    fprintf(fout, "%d\n", nnumbers);
    
    /* write numbers to fout */
    for(uint32_t i = 0; i < nnumbers; ++i) {
        fprintf(fout, "%d\n", numbers[i]);
    }
    fclose(fout);
}
static inline double monotonic_seconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
static void print_time(double const seconds)
{
    printf("Sort Time: %0.04fs\n", seconds);
}
int cnt=0;
void Qsort(int* A,int l,int r)
{
    if(l<r)
    {
        int pivit = A[r];
        int i,j;
        for(i=l-1,j=l;j<=r-1;j++){
            if(A[j]<=pivit){
                i++;
                swap(A[i],A[j]);
            }
            ++cnt;
        }
        swap(A[i+1],A[r]);
        Qsort(A,l,i);
        Qsort(A,i+2,r);
    }
}

int main() {
    srand(clock());
    A = (int*)malloc(N*sizeof(int));
    for(int i=1;i<=N;i++)
        //A[i]=i;
        A[i]=(rand()%N);
    double START = monotonic_seconds();
    //sort(A+1,A+1+N);
    Qsort(A,1,N);
    double END = monotonic_seconds();
    print_time(END - START);
    for(int i=1;i<N;i++) if(A[i]>A[i+1]) printf("no at %d\n",i);
    printf("%N = %d, cnt = %d\n",N,cnt);
    return 0;
}

