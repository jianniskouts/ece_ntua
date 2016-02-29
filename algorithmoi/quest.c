#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
 
#define minN 2
#define maxN 1000
#define minM 2
#define maxM 1000
#define minK 1
#define maxK 200
 
#define FIRST       1
#define SECOND      2
#define ADVERSARY   3
 
/* Globally used variables */
long int dungeon[maxN + 1][maxM + 1];
long long int memoFirst[maxN + 1][maxM + 1], memoSecond[maxN + 1][maxM + 1], memoAdversary[maxN + 1][maxM + 1];
 
int N, M, K;
int adversaryStartX, adversaryStartY;
 
#ifdef DEBUG
    int recursionDepth = 0;
#endif
 
/* Function Declarations */
void read_input(long int argc, char *argv[]);
long long int first(int x, int y);
long long int second(int x, int y);
long long int check_memo(int x, int y, int memo);
int store_memo(int x, int y, long long int value, int memo);
 
int main(int argc, char *argv[]) {
    int i, j;
 
    read_input(argc, argv);
     
    memset(memoFirst, -1, sizeof(memoFirst));
    memset(memoSecond, -1, sizeof(memoSecond));
    memset(memoAdversary, -1, sizeof(memoAdversary));
 
    /* find best strategy to maximize sales */
    printf("%lld ", first(1, 1));
    printf("%lld\n", memoAdversary[adversaryStartX][adversaryStartY]);
    return 0;
}
 
/* Maximum gold coins that you can collect if the current position is at room (x, y) and you play first */
long long int first(int x, int y) {
    int p, q, newx, newy;
    long long int max = -1, current, adversaryCurrent, adversaryNew;
     
    #ifdef DEBUG
        recursionDepth++;
        printf("%d. first(%d, %d)\n", recursionDepth, x, y);
    #endif
    /* Out of bounds. Stop recursion */
    if (x > N || y > M) {
        #ifdef DEBUG
            recursionDepth--;
        #endif
        return 0;
    }
     
    /* first check if we have already computed the requested value */
    max = check_memo(x, y, FIRST);
    if (max != -1) {
        #ifdef DEBUG
            printf("Value found in memo\n");
            recursionDepth--;
        #endif
        return max;
    }
     
    /* else compute the requested value */
    for (p = 0; p <= K; p++) {
        for (q = 0; q <= K; q++) {
            if (p + q == 0) {
                continue;   /* a move must be made */
            }
            current = dungeon[x + p][y + q] + second(x + p, y + q);
            if (current > max) {
                max = current;
                newx = x + p;
                newy = y + q;
            }
            else if (current == max) {
                adversaryCurrent = first(newx, newy);
                adversaryNew = first(x + p, y + q);
                if (adversaryNew < adversaryCurrent) {
                    max = current;
                    newx = x + p;
                    newy = y + q;
                }
            }
        }
    }
     
    /* and store it for further use */
    if (!store_memo(x, y, max, FIRST)) {
        printf("ERROR: Failed to store value!");
        exit(1);
    }
     
    /* Starting position for adversary */
    if ( x == 1 && y == 1) {
        adversaryStartX = newx;
        adversaryStartY = newy;
        #ifdef DEBUG
            printf("adversaryStartX = %d, adversaryStartY = %d\n", adversaryStartX, adversaryStartY);
        #endif
    }
    #ifdef DEBUG
        recursionDepth--;
    #endif
    return max;
}
 
/* Maximum gold coins that you can collect if the current position is at room (x, y) and you play second */
long long int second(int x, int y) {
    int p, q, newx, newy;
    long long int max = -1, current, adversaryCurrent, adversaryNew;
     
    #ifdef DEBUG
        printf("second(%d, %d)\n", x, y);
    #endif
    /* Out of bounds. Stop recursion */
    if (x > N || y > M) {
        return 0;
    }
     
    /* first check if we have already computed the requested value */
    max = check_memo(x, y, SECOND);
    if (max != -1) {
        #ifdef DEBUG
            printf("Value found in memo\n");
        #endif
        return max;
    }
     
    /* else compute the requested value */
    for (p = 0; p <= K; p++) {
        for (q = 0; q <= K; q++) {
            if (p + q == 0) {
                continue;   /* a move must be made */
            }
            current = dungeon[x + p][y + q] + second(x + p, y + q);
            if (current > max) {
                max = current;
                newx = x + p;
                newy = y + q;
            }
            else if (current == max) {
                adversaryCurrent = first(newx, newy);
                adversaryNew = first(x + p, y + q);
                if (adversaryNew < adversaryCurrent) {
                    max = current;
                    newx = x + p;
                    newy = y + q;
                }
            }
        }
    }
 
    /* store adversary's result */
    if (!store_memo(x, y, max, ADVERSARY)) {
        printf("ERROR: Failed to store value!");
        exit(1);
    }
     
    max = first(newx, newy);
     
    /* store value for further use */
    if (!store_memo(x, y, max, SECOND)) {
        printf("ERROR: Failed to store value!");
        exit(1);
    }
    return max;
}
 
/* If found, return requested value, else return -1 */
long long int check_memo(int x, int y, int memo) {
    switch (memo) {
    case FIRST:
        if (memoFirst[x][y] != -1) {
            return memoFirst[x][y];
        }
        break;
    case SECOND:
        if (memoSecond[x][y] != -1) {
            return memoSecond[x][y];
        }
        break;
    }
    return -1;
}
 
/* On success return 1, else return 0 */
int store_memo(int x, int y, long long int value, int memo) {
    switch (memo) {
    case FIRST:
        memoFirst[x][y] = value;
        return 1;
        break;
    case SECOND:
        memoSecond[x][y] = value;
        return 1;
        break;
    case ADVERSARY:
        memoAdversary[x][y] = value;
        return 1;
        break;
    }
    return 0;
}
 
void read_input(long int argc, char *argv[]) {
    int i, j;
     
    #ifdef DEBUG
        /* In DEBUG MODE read from file */
        if(argc<2) {
            printf("Usage:./quest <input.txt>\n");
            exit(1);
        }
        FILE *fp = fopen(argv[1],"r");
         
        /* Read N, M, K */
        if(EOF == fscanf(fp,"%d %d %d",&N, &M, &K)) {
            printf("Error: Reading N, M, K\n");
            exit(1);
        }
    #else
        scanf("%d %d %d",&N, &M, &K);
    #endif
     
    memset(dungeon, 0, sizeof(dungeon));
 
    for(i = 1;i <= N; i++) {
        for(j = 1;j <= M; j++) {
            #ifdef DEBUG
                if(EOF == fscanf(fp, "%ld", &dungeon[i][j])) {
                    printf("Error: Reading dungeon[%d][%d]\n", i, j);
                    exit(1);
                }
            #else
                scanf("%ld", &dungeon[i][j]);
            #endif
        }
    }
     
    #ifdef DEBUG
        printf("Input:\n");
        printf("%d %d %d\n", N, M, K);
        for(i = 1;i <= N; i++) {
            for(j = 1;j <= M; j++) {
                printf("%5ld ", dungeon[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    #endif
    return;
}