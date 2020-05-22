#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include "list.h"
// #include "transposition.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

#define MAX_DEPTH 8
#define INFINITY 999999999

#define MAX_TIME 7
/**********************************************************/
Position gamePosition;		// Position we are going to use

//Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

unsigned char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "Stefanos";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/

//Execute client using valgrind to find any leaks
// valgrind --leak-check=full \
//          --show-leak-kinds=all \
//          --track-origins=yes \
//          --verbose \
//          --log-file=valgrind-out.txt  ./client


int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove);


Move* findMove(Position*, int);
int evalPosition(Position*);
int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}



	connectToTarget( port, ip, &mySocket );

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server indorms us that we have WHITE color
				myColor = WHITE;
				printf("My color is %d\n",myColor);
				break;

			case NM_COLOR_B:			//server indorms us that we have BLACK color
				myColor = BLACK;
				printf("My color is %d\n",myColor);
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{


					int maxScore = -INFINITY;

					Position* tempPosition = malloc(sizeof(Position));
					memcpy(tempPosition, &gamePosition, sizeof(Position));

					maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, &myMove);

					printf("\t\tMAX SCORE %d\n", maxScore);

					free(tempPosition);

					//THIS ALSO WORKS
					//maxScore = alpha_beta(&gamePosition, MAX_DEPTH, maxScore, -maxScore, 1, &myMove);



				}
				//AFTER THE UPDATED VERSION OF ASSIGNMENT CHANGED THOSE
				sendMove( &myMove, mySocket );			//send our move
				// printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				printPosition( &gamePosition );

				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
			default:
				printf("Wrong Signal!\n");
				return 0;
		}

	} 
	return 0;
}


void recursiveFollowJump(list* moveList, Move* move, int k /* depth of recursion*/,char i, char j, Position *aPosition){
	
	int possibleJumps, playerDirection;
	char color = move->color;
	move->tile[0][k] = i;
	move->tile[1][k] = j;

	possibleJumps = canJump(i, j, color, aPosition);


	playerDirection = aPosition->turn == WHITE ? 1 : -1;

	if(possibleJumps){
		if(possibleJumps == 1) //can jump left
		{
			recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
			return;
		}else if(possibleJumps == 2) //can jump right
		{
			recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j+2, aPosition);
			return;
		}else if(possibleJumps == 3) //we need to split the jumps
		{
			//copying move:
			Move * newMove = malloc(sizeof(Move));
			memcpy(newMove, move, sizeof(Move));
			//following both left and right
			recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
			recursiveFollowJump(moveList, newMove, k+1, i + 2*playerDirection, j+2, aPosition);
			return;

		}
	}


	if( k + 1 != MAXIMUM_MOVE_SIZE ){
		move->tile[0][k+1] = -1;
	}


	if(isLegal(aPosition, move)){
		push(moveList, move);
		}
	else
		free(move);
	return;
	
}

list* findAllMoves(Position *aPosition) {
	int i, j, jumpPossible = 0, movePossible = 0, playerDirection;
	list* moveList = (list*)malloc(sizeof(list));

	initList(moveList);
	Move *move;

	playerDirection = aPosition->turn == WHITE ? 1 : -1; 


	//Start iterating through the board to track all available moves
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++)
			{
				if( aPosition->board[ i ][ j ] != aPosition->turn ) continue;
					
					//From assignment we give priority to jumps than simple moves
					if( canJump( i, j, aPosition->turn, aPosition ) ){
						//printf("JUMP POSSIBLE\n");
						if(!jumpPossible) freeList(moveList); //any simple moves are deleted
						
						move = malloc(sizeof(Move));
						memset(move,0,sizeof(Move));
						move->color = aPosition->turn;
						recursiveFollowJump(moveList, move, 0, i, j, aPosition); //FOUND! IF MORE THAN ONE JUMP POSSIBLE THEN PUSHING THE SAME MALLOC'd move
						jumpPossible = 1;
					}else if((jumpPossible == 0)){ //We come in here only when we have not found yet a jump(only simple moves)

						move = (Move*)malloc(sizeof(Move));
						move->color = aPosition->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j-1;
						move->tile[0][2] = -1;

						if(isLegal(aPosition, move)){
							push(moveList, move);}
						else
							free(move);

						move = (Move*) malloc(sizeof(Move));
						move->color = aPosition->turn;
						move->tile[0][0] = i;
						move->tile[1][0] = j;
						move->tile[0][1] = i + playerDirection;
						move->tile[1][1] = j+1;
						move->tile[0][2] = -1;
						if(isLegal(aPosition, move)){
							push(moveList, move);}
						else
							free(move);

					}
				
			}
	}

	// if(moveList==NULL){ //if we can't move
	// 	move = malloc(sizeof(Move));
	// 	move->color = aPosition->turn;
	// 	move->tile[0][0] = -1;
	// 	push(moveList, move);
	// 	return moveList;
	// }
	return moveList;
						
}

//TODO will change that eventually
int evalPosition (Position *aPosition) {
    int i,j, evaluation = 0;
    
    for (i = 0; i < BOARD_ROWS; i++)
    {
        for ( j = 0; j < BOARD_COLUMNS; j++)
        {
            if (aPosition->board[i][j] == myColor){
               evaluation += 100;
               //printf("For piece in %d %d, of color: %d, we add...\n", i, j, myColor);
               if(myColor == WHITE)
               	evaluation += i*1;
               else
               	evaluation += (BOARD_ROWS-i-1)*1;


           	}
            else if (aPosition->board[i][j] == getOtherSide(myColor)){
               evaluation -= 100;
               //printf("For piece in %d %d, of color: %d, we sub...\n", i, j, getOtherSide(myColor));
               if(myColor == BLACK)
               	evaluation -= i*1;
               else
               	evaluation -= (BOARD_ROWS-i-1)*1;
           	}

        }
    }
    evaluation = evaluation + aPosition->score[myColor]*90 - aPosition->score[getOtherSide(myColor)]*90;

    return evaluation;
}
int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}
int min(int a, int b)
{
	if(a < b)
		return a;
	return b;
}


int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove){  //recursive minimax function.
	

	if (depth <= 0){ //if we reached the maximum depth of our recursion and there are no captures we can see
			return evalPosition(aPosition); //return heuristic
	}

	list *moveList = findAllMoves(aPosition);   //finding all legal moves in this position
	Move *tempData = NULL;


	if (moveList == NULL || moveList->head == NULL){     //If for any reason, no more moves are available
		freeList(moveList);
		return evalPosition(aPosition);
	}

	Position* tempPosition = malloc(sizeof(Position));
	int tempScore, g;

	int a, b;


	if (maximizingPlayer){

		g = -INFINITY;
		a = alpha;
		while((g<beta)&&((tempData = pop(moveList)) != NULL)){ //for each child position


			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);

			tempScore = alpha_beta(tempPosition, depth-1, a, beta, 0, NULL);

			//g = max(g, tempScore);
			

			if(g < tempScore){
				g = tempScore;
				if(finalMove != NULL){
					//printf("%d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
					memcpy(finalMove, tempData, sizeof(Move));
				}
			}
			a = max(a, g);
			//alpha = max(alpha, tempScore);
			//if( beta <= alpha){ free(tempData); break;}
			free(tempData);
		}/*
		freeList(moveList);
		free(tempPosition);

		return alpha;*/
	}else{
		g = INFINITY;
		b = beta;
		while((g>alpha)&&(tempData = pop(moveList)) != NULL){ //for each child position

			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);
			//printf("Searching Move on depth %d:\n", depth);
			//printMove(tempData);
			tempScore = alpha_beta(tempPosition, depth-1, alpha, b, 1, NULL);
			
			g = min(g, tempScore);
			b = min(b, g);

			//if( beta <= alpha){ free(tempData); break;}
			free(tempData);
		}
		/*

		return beta;
		*/
	}

	freeList(moveList);
	free(tempPosition);
	return g;
}
