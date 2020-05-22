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

#define MAX_DEPTH 5
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
					Move *tempMove = findMove(&gamePosition, 0/*initial depth*/);
					myMove = *tempMove;
					free(tempMove);
				}

				sendMove( &myMove, mySocket );			//send our move
				//printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				//doMove( &gamePosition, &myMove );		//play our move on our position
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
	
	assert(MAXIMUM_MOVE_SIZE > k);
	
	int possibleJumps, playerDirection;
	char color = move->color;
	move->tile[0][k] = i;
	move->tile[1][k] = j;

	if(!(possibleJumps = canJump(i, j, color, aPosition))){
		move->tile[0][k+1] = -1;
		//assert(isLegal(aPosition, move));
		if(isLegal(aPosition, move)){

			push(moveList, move);
			}
		else
			free(move);
		return;
	}

	if( color == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;


	if(possibleJumps == 1) //can jump left
	{
		recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
	}


	if(possibleJumps == 2) //can jump right
	{
		recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j+2, aPosition);
	}

	if(possibleJumps == 3) //we need to split the jumps
	{
		//copying move:
		Move * newMove = malloc(sizeof(Move));
		memcpy(newMove, move, sizeof(Move));
		//following both left and right
		recursiveFollowJump(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
		recursiveFollowJump(moveList, newMove, k+1, i + 2*playerDirection, j+2, aPosition);

	}
	return;
}
list* findAllMoves(Position *aPosition) {
	int i, j, jumpPossible = FALSE, movePossible = FALSE, playerDirection;
	list* moveList = malloc(sizeof(list));

	initList(moveList);
	Move *move;

	char curColor = aPosition->turn;

	if( curColor == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;





	//jump not possible, finding all normal moves
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++)
			{
				if( aPosition->board[ i ][ j ] == curColor )
				{
					if( canJump( i, j, curColor, aPosition ) ){
						if(!jumpPossible)
							emptyList(moveList); //any simple moves are deleted
						move = malloc(sizeof(Move));
						move->color = curColor;
						recursiveFollowJump(moveList, move, 0, i, j, aPosition); //FOUND! IF MORE THAN ONE JUMP POSSIBLE THEN PUSHING THE SAME MALLOC'd move
						jumpPossible = TRUE;
					}
					if((jumpPossible == FALSE))
					{
						// if(movePossible % 2 == 1) //left move possible
						// {	
							move = malloc(sizeof(Move));
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
						// }
						// if(movePossible > 1){
							move = malloc(sizeof(Move));
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
						// }

					}

				}
			}
	}

	if(top(moveList)==NULL){ //if we can't move
		move = malloc(sizeof(Move));
		move->color = curColor;
		move->tile[0][0] = -1;
		push(moveList, move);
		return moveList;
	}
	return moveList;
						
}

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


int quiescenceSearch(Position* aPosition){
		int i, j;
		//quiescence search
		// determine if we have a jump available
		for( i = 0; i < BOARD_ROWS; i++ )
		{
			for( j = 0; j < BOARD_COLUMNS; j++ )
			{
				if( aPosition->board[ i ][ j ] == aPosition->turn )
				{
					if( canJump( i, j, aPosition->turn, aPosition ) ){
						return TRUE;
						}
				}
			}
		}
		return FALSE;


}

int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove){  //recursive minimax function.
	

	if (depth <= 0){ //if we reached the maximum depth of our recursion
		//if(!quiescenceSearch(aPosition)){ // and there are no captures we can see
			return evalPosition(aPosition); //return heuristic
		//}
		
	}

	list *moveList = findAllMoves(aPosition);   //finding all legal moves in this position
	Move * tempData = NULL;


	if (top(moveList) == NULL){     //If for any reason, no more moves are available
		freeList(moveList);
		return evalPosition(aPosition);
	}

	Position* tempPosition = malloc(sizeof(Position));
	//Move *tempMove = malloc(sizeof(Move));
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
		saveTransposition(aPosition, alpha, depth);
		//printf("alpha: %d\n", alpha);

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

		saveTransposition(aPosition, beta, depth);
		//printf("%d\n", beta);
		return beta;
		*/
	}

	freeList(moveList);
	free(tempPosition);
	
	
	

	return g;
}



Move* findMove(Position* aPosition, int depth){

	
	Move* myFinalMove = malloc(sizeof(Move));

	Position* tempPosition = malloc(sizeof(Position));
	memcpy(tempPosition, aPosition, sizeof(Position));

	int maxScore = -INFINITY;
	printf("Cur position evaluation: %d\n", evalPosition(aPosition));
	maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, myFinalMove);
	//maxScore = MTFD(tempPosition, evalPosition(aPosition), MAX_DEPTH, myFinalMove);
	//iterativeDeepening(tempPosition, myFinalMove);
	
	//freeList(moveList);
	free(tempPosition);
	//printList(moveList);
	
	return myFinalMove;
}

