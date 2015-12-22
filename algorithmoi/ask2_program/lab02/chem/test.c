#include <stdio.h>
#include <stdlib.h>

static int A[1500][1500];
int E(int i, int l);
int cost(int j, int i);

int cost(int j, int i){
	int sum=0, p, q;
	for (p=j; p<i; p++){
		for (q=p+1; q<=i; q++){
			sum = sum + A[p][q];
		}
	}
	return sum;
}

int E(int i, int l){
	int j, temp; 
	if (i<=l || i ==1){
		return 0;
	}
	if (l==1){
		return cost(1,i);
	}	
	//if (l>1){
		int min=1000000000;
		for(j=1; j<i-1; j++){
			temp = E(j,l-1) + cost(j+1,i);
			printf("%d\n",j );
			//printf("temp= %i \n", temp);
			if(temp<min){
				min = temp;
			}
		}
		return min;
	//}
	
}

/*long long int E (long long int i, long long int l){
	long long int enrgy,j,min;
	if (i == l || i == 1) return 0;
	if (l==1) return cost(1,i);
	for (j=1;j<=i-1;j++){
		if (j==1) min = E(j,l-1) + cost(j+1,i);
		if ((E(j,l-1)+cost(j+1,i))<min) min = E(j,l-1)+cost(j+1,i);
	}
	return min;
	//to endexomeno na exw perissotera doxeia ??
}*/

int main(){
	int i, j, energy, N, K;
	scanf("%d %d",&N,&K);
	for(i=1; i<N; i++)
	{
		A[i][i]=0;
		for(j=i+1; j<N+1; j++)
		{
			scanf("%d\n", &A[i][j]);
			A[j][i]=A[i][j];
		}
	}
	energy = E(N,K);
	printf("%d\n", energy);

	return 0;
}