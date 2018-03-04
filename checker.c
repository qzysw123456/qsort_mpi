#include <stdio.h>
int main(int argc,char* argv[]) {
    char* file = argv[1];
    FILE* in = fopen(file,"r");
    int N;
    fscanf(in, "%d",&N);
    int prev;
    int flag = 1;
    for(int i = 0;i<N;i++){
        int now;
        fscanf(in, "%d",&now);
        if(i){
            if(now<prev) flag = 0;
        }
        prev = now;
    }
    if(flag ==0)
        printf("Fault\n");
    else
        printf("Correct\n");
    return 0;
}
