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
    int pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element
    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
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
    return i;
}

//int LeftChild(int x){
//    return x<<1|1;
//}
//void PercDown(int* A, int i, int N)
//{
//    int child;
//    int tmp;
//    for (tmp = A[i]; LeftChild(i)<N; i = child)
//    {
//        child = LeftChild(i);
//        if (child != N-1 && A[child+1]>A[child])
//            child++;
//        if (A[i]<A[child])
//            Swap(&A[i],&A[child]);
//        else
//            break;
//    }
//}
//void HeapSort(int* A, int N)
//{
//    int i;
//    for (i  = N/2;  i>=0; i--)
//        PercDown(A, i, N);
//    for ( i = N-1; i > 0; i--)
//    {
//        Swap(&A[0], &A[i]);
//        PercDown(A, 0, i);
//    }
//}
void quickSort(int arr[], int low, int high)
{
    /*bool flag = true;
     for(int i = low; i < high ; i++) if(arr[i]>arr[i+1]) {flag = false;break;}
     if(flag) return;*/
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
         at right place */
        int pi = partition(arr, low, high);
        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}
//int binary_find(int arr[],int low,int high,int x)
//{
//    while(low<high){
//        int mid = (low+high)>>1;
//        if(arr[mid]<x){
//            low = mid + 1;
//        }
//        else
//            high = mid;
//    }
//    return low;
//}
//void introsort(int* A,int l,int r,int Depth)
//{
//    int len = r - l + 1;
//    if(len<=1)
//        return;
//    else if(Depth==0){
//        HeapSort(A+l, len);
//    }
//    else{
//        int p = partition(A,l,r);
//        introsort(A, l, p-1, Depth -1 );
//        introsort(A, p+1, r, Depth -1 );
//    }
//}
//void Sort(int* A,int l,int r)
//{
//    int maxLen =floor(log2(r-l+1))*2;
//    introsort(A, l, r, maxLen);
//}
int LOG(int x)
{
    int cnt = 0;
    while(x){
        cnt++;
        x>>=1;
    }
    return cnt;
}
int A[100000000];
int B[100000000];
int main(int argc,char** argv) {
    int num_procs, myid, name_len;
    int world_rank,world_size;
    char proc_name[MPI_MAX_PROCESSOR_NAME];
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
    
    /* Obtain name of machine process is executing on */
    MPI_Get_processor_name(proc_name, &name_len);
    
    //printf("Hello World from processor %d out of %d, executing on %s, world_rank %d,size %d\n", myid, num_procs, proc_name,world_rank,world_size);
    
    int level = LOG(num_procs);
    
    MPI_Comm* NewComm;
    NewComm = malloc((1+LOG(num_procs))*sizeof(MPI_Comm));
    NewComm[LOG(num_procs)] = MPI_COMM_WORLD;
    
    int* Pivit_set = NULL;
    int Comm_Pivit;
    int SendNum;
    int RecvNum;
    while(level != 1){
        //Sort(A,0,BlockSize-1);
        //for(int i=0;i<BlockSize;i++){
        //    printf("%d ",A[i]);
        //}
        //printf("I am %d \n",world_rank);
        
        //int Pivit = A[BlockSize/2];
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
        //printf("i am %d my pivit is %d comm pivit is %d\n",world_rank,Pivit,Comm_Pivit);
        //int pos = binary_find(A,0,BlockSize,Comm_Pivit);
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
        //printf("send %d recv%d,from processor %d out of %d, executing on %s, world_rank %d,size %d\n",SendNum,RecvNum, myid, num_procs, proc_name,world_rank,world_size);
    }
    quickSort(A,0,BlockSize-1);
    //MPI_Gather(A,BlockSize,MPI_INT,B,1,MPI_INT,0,MPI_COMM_WORLD);
    
    
//    for(int i=0;i<BlockSize;i++){
//        printf("%d ",A[i]);
//    }
    printf("world = %d ,BlockSize = %d over\n",world_rank,BlockSize);
    

    int RecvBlockSize;
    if(world_rank == 0){
        //print_time(End-Start);
        for(int i=1;i<world_size;i++){
            MPI_Recv(&RecvBlockSize,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(A+BlockSize,RecvBlockSize,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            BlockSize += RecvBlockSize;
        }
        double End = monotonic_seconds();
        print_time(End-Start);
        //print_numbers(argv[2],A,N);
    }
    else{
        MPI_Send(&BlockSize,1,MPI_INT,0,0,MPI_COMM_WORLD);
        MPI_Send(A,BlockSize,MPI_INT,0,0,MPI_COMM_WORLD);
    }
    
    /* Last call to MPI (REQUIRED) */
    MPI_Finalize();
    return 0;
}

