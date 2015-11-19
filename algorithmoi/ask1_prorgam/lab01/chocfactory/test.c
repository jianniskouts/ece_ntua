#include<stdio.h>
#include<stdlib.h>
void quicksort(long int x[100000], long int, long int);
void haris(long int x[],long int y[],long int N,long int K);
int main(){
    
    long int x[100000],y[100000],N,i,K;
    scanf("%li",&N);
    scanf("%li",&K);
    for(i=0;i<N;i++){
      scanf("%li",&x[i]);
      scanf("%li",&y[i]);
    }

    quicksort(x,0,N-1);
    quicksort(y,0,N-1);

    haris(x,y,N,K);
    return 0;
}

void haris(long int x[], long int y[], long int N, long int K){
    long int D = 0;
    int counter = 0;  //metritis litrwn ana xroniki stigmi 
    int i = 0;  //metritis pinaka timwn enarksis
    int j = 0;  //metritis pinaka timwn liksis
    int l = 0;  //metritis xronikwn timwn
    int to_be = 0;  //metritis litrwn ana xroniki stigmi me ti meiwsi ston kleisimo tis vrisis
    while(D < K){
        counter = to_be;  
        while ((i < N) && (x[i] == l)){ //elegxos an anoigei mia vrisi
              counter++; 
              to_be++;
              i++   ;
      }
        while ((j < N) && (y[j] == l)){ //elegxos an kleinei mia vrisi
              to_be--;
              j++   ;
      }
        D = D + counter;
        l++; 
    } 
    l--;
            printf("%d\n",l);
}
    
    
void quicksort(long int x[100000],long int first,long int last){  //quicksort twn duo pinakwn
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