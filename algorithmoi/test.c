#include<stdio.h>
#include<stdlib.h>
void quicksort(long int x[100000], long int, long int);
void haris(long int x[],long int y[],long int N,long int K);
int main(){
    
    long int x[100000],y[100000],N,i,K;
    scanf("%d",&N);
    scanf("%d",&K);
    for(i=0;i<N;i++){
      scanf("%d",&x[i]);
      scanf("%d",&y[i]);
    }

    quicksort(x,0,N-1);
    quicksort(y,0,N-1);

    haris(x,y,N,K);
    return 0;
}

void haris(long int x[], long int y[], long int N, long int K){
    long int D = 0;
    int counter = 0;
    int i = 0;
    int j = 0;
    int l = 0;
    int to_be = 0;
    while(D < K){
        counter = to_be;
        while ((i < N) && (x[i] == l)){
              //printf("x = %li\n",x[i] );
              //printf("i = %d\n",i );
              counter++; 
              to_be++;
              i++   ;
      }
        while ((j < N) && (y[j] == l)){
              //printf("x = %li\n",y[j] );
              //printf("j = %d\n",j ); 
              to_be--;
              j++   ;
      }

        //printf("counter = %d\n",counter );
        //printf("to_be = %d\n",to_be );
        D = D + counter;
        l++; 
    } 
    l--;
            printf("l = %d, D = %d\n",l,D);
}
    
    

void quicksort(long int x[100000],long int first,long int last){
    int pivot,j,temp,i;

     if(first<last){
         pivot=first;
         i=first;
         j=last;

         while(i < j){
             while((x[i] <= x[pivot]) && (i < last))
                 i++;
             while(x[j] > x[pivot])
                 j--;
             if(i < j){
                 temp = x[i];
                  x[i] = x[j];
                  x[j] = temp;
             }
         }

         temp = x[pivot];
         x[pivot] = x[j];
         x[j] = temp;
         quicksort(x,first,j-1);
         quicksort(x,j+1,last);

    }
}