#include <stdio.h>
#include <stdlib.h>


int main(){
	int i, j, k, z,  N, K, paral;
	int counter = 0; 
	int A[700][700];
	int B[700][700];
	scanf("%d", &N);
	scanf("%d",&K);
	int c;
	

	c = getchar();
	for(i = 0; i < N; i++){
		for(j = 0; j < N+1; j++){
			if ( (c = getchar()) != '\n')
			{
				A[i][j] = c - 48;
			}
		}
	}
	counter = 0;
	paral = 0;
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			
			for (k = i; k < N; k++){
				for (z = j; z < N; z++){
					if ((k == i ) || (z == j)){
							if ((k == i) && (z  == j)){
								B[k-i][z-j] = A[k][z];
								
								//printf(" b = %d (x,y) = (%d,%d) a = %d \n",B[k-i][z-j],k,z,A[z][k] );
							}
							else if ((k == i ) && (z  != j)){
								B[k-i][z-j] = B[k-i][z-j-1] + A[k][z];
							}
							else{
								B[k-i][z-j] = B[k-i-1][z-j] + A[k][z];
							}
					}
					else{
						B[k-i][z-j] = B[k-i-1][z-j-1] + (B[k-i][z-j-1] - B[k-i-1][z-j-1]) + (B[k-i-1][z-j] - B[k-i-1][z-j-1]) + A[k][z];
						
					}
					//printf(" B = %d (x,y) = (%d,%d) \n",B[k-i][z-j],k,z );
				//y++;
					if(B[k-i][z-j] == K){
							counter++;
						}
				}
				//x++;
			}
			//printf("\n");
		}
	}
	/*printf("B = \n");
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			printf("%d", B[i][j] );
		}
			printf("\n ");
		
	}
	*/
	printf("%d\n",counter );
	return 0;
}
