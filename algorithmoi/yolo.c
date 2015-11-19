#include <stdio.h>
#include <stdlib.h>

int main(){
	int i, j, k, z, x, c, p, N, K, m=0, assoi, paral, counter; 
	int A[700][700];
	scanf("%d", &N);
	scanf("%d", &K);
		
	for(i=0; i<N; i++){
		for(j=0; j<N+1; j++){
			//scanf("%d", &A[i][j]);
			A[i][j]=getchar();
		}
	}
	for(i=0; i<N; i++){
		for(j=0; j<N+1; j++){
			//putchar(A[i][j]);
			printf("%c", A[i][j]);
		}
		
	}
	printf("\n");
	
	
	counter=0;
	paral=0;
	for (i=0; i<N; i++){
		for (j=0; j<N; j++){
			for (k=i; k<N; k++){
				for (z=j; z<N; z++){
					assoi=0;
					for(x=i; x<=k; x++){
						for(c=j; c<=z; c++){
							if (A[x][c]==1){
								assoi++;
							}
							if(assoi>K){
								break;
							}
						}
						if(assoi>K){
							break;
						}
					}
					if (assoi==K){
						counter++;
					}
					//paral++;
				}
			}
		}
	}
	printf("counter=%d\n", counter);
	//printf("assoi=%d\n", assoi);
	printf("parallilogramma=%d\n", paral);
	
	return 0;
}
