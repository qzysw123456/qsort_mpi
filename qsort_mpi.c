#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h>
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
void Swap(int* x,int* y)
{
    int tmp = *x;
    *x = * y;
    *y = tmp;
}
int mediumTri(int* A,int x,int y,int z)
{
    if(A[x]>A[y]) Swap(&x, &y);
    if(A[y]>A[z]) Swap(&y, &z);
    if(A[x]>A[y]) Swap(&x, &y);
    return y;
}
int partition (int arr[], int low, int high)
{
    int select = mediumTri(arr, low, high, (low+high)>>1);
    Swap(&arr[select], &arr[high]);
    int pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high- 1; j++)
    {
        if (arr[j] <= pivot)
        {
            i++;
            Swap(&arr[i], &arr[j]);
        }
    }
    Swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
int Partition(int arr[],int low,int high,int pivit)
{
    int i = low - 1;
    for(int j = low ; j <= high; j++){
        if(arr[j]<=pivit){
            i++;
            Swap(&arr[i],&arr[j]);
        }
    }
    return i+1;
}
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int A[100000000];
int B[100000000];
int main(int argc,char** argv) {
    int num_procs, myid;
    int world_rank,world_size;
    int N;
    N = atoi(argv[1]);
    
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    int BlockSize = N/world_size;
    srand(myid);
    for(int i = 0;i < BlockSize; i++ )
        A[i] = rand();
    
    double Start = monotonic_seconds();
    int level = log2(num_procs)+1;
    
    MPI_Comm* NewComm;
    NewComm = malloc((2+log2(num_procs))*sizeof(MPI_Comm));
    NewComm[(int)log2(num_procs)+1] = MPI_COMM_WORLD;
    int* Pivit_set = NULL;
    int Comm_Pivit;
    int SendNum;
    int RecvNum;
    while(level != 1){
        int Pivit = A[rand()%BlockSize];
        if( myid == 0) {
            Pivit_set = malloc(num_procs*sizeof(int));
        }
        MPI_Gather(&Pivit,1,MPI_INT,Pivit_set,1,MPI_INT,0,NewComm[level]);
        if(myid==0){
            quickSort(Pivit_set,0,num_procs-1);
            Comm_Pivit = Pivit_set[num_procs/2];
        }
        MPI_Bcast(&Comm_Pivit,1,MPI_INT,0,NewComm[level]);
        int pos = Partition(A,0,BlockSize-1,Comm_Pivit);
        if(myid>=num_procs/2){
            //before pos
            int RecID = myid - num_procs/2;
            SendNum = pos;
            
            MPI_Send(&SendNum,1,MPI_INT,RecID,0,NewComm[level]);
            MPI_Send(A,SendNum,MPI_INT,RecID,0,NewComm[level]);
            
            MPI_Recv(&RecvNum,1,MPI_INT,RecID,0,NewComm[level],MPI_STATUS_IGNORE);
            MPI_Recv(B,RecvNum,MPI_INT,RecID,0,NewComm[level],MPI_STATUS_IGNORE);
            
            memcpy(A,A+pos,(BlockSize-pos)*sizeof(int));
            memcpy(A+BlockSize-pos,B,RecvNum*sizeof(int));
            
        }
        if(myid<num_procs/2){
            //>=pos
            int RecID = myid + num_procs/2;
            
            MPI_Recv(&RecvNum,1,MPI_INT,RecID,0,NewComm[level],MPI_STATUS_IGNORE);
            MPI_Recv(B,RecvNum,MPI_INT,RecID,0,NewComm[level],MPI_STATUS_IGNORE);
            
            SendNum = BlockSize - pos;
            MPI_Send(&SendNum,1,MPI_INT,RecID,0,NewComm[level]);
            MPI_Send(A+pos,SendNum,MPI_INT,RecID,0,NewComm[level]);
            
            memcpy(A+pos,B,RecvNum*sizeof(int));
        }
        BlockSize = BlockSize - SendNum + RecvNum;
        MPI_Comm_split(NewComm[level], myid>=num_procs/2, myid, &NewComm[level-1]);
        MPI_Comm_rank(NewComm[level-1], &myid);
        MPI_Comm_size(NewComm[level-1], &num_procs);
        --level;
    }
    quickSort(A,0,BlockSize-1);

    //printf("world = %d ,BlockSize = %d over\n",world_rank,BlockSize);
    int RecvBlockSize;
    if(world_rank == 0){
        //print_time(End-Start);
        for(int i=1;i<world_size;i++){
            MPI_Recv(&RecvBlockSize,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(A+BlockSize,RecvBlockSize,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            BlockSize += RecvBlockSize;
        }
    }
    else{
        MPI_Send(&BlockSize,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(A,BlockSize,MPI_INT,0,0,MPI_COMM_WORLD);
    }
    MPI_Scatter(A,N/world_size,MPI_INT,A,N/world_size,MPI_INT,0,MPI_COMM_WORLD);
   
    if(world_rank == 0){
        double End = monotonic_seconds();
        print_time(End-Start);
        print_numbers(argv[2],A,N);
    }
    /* Last call to MPI (REQUIRED) */
    MPI_Finalize();
    return 0;
}

