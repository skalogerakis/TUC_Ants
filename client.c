#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "client.h"
#include <string.h>

#define INFINITY 999999999
#define MAX_DEPTH 5
/**********************************************************/
Position gamePosition;		// Position we are going to use

//Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "Stefanos";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/


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

	srand( time( NULL ) );

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
					printf("MAIN no available moves\n");
				}
				else
				{
					// short choice = 1;

					// switch(choice){
					// 	case 1: 
					//printf("Cur position evaluation: %d\n", evaluationFunction(&gamePosition));
							myMove = initRandom(myColor,&gamePosition);
						// 	printf("Random Choice 1\n");
						// 	break;
						// default: 
						 	//myMove = initRandom(myColor);
						// 	printf("Default choice\n");
						// 	break;
					// }
					// Move *tempMove = newRandom(myColor,&gamePosition);
					// myMove = *tempMove;
					// free(tempMove);





					// int maxScore = -INFINITY;
					// Move* myFinalMove = malloc(sizeof(Move));

					// Position* tempPosition = malloc(sizeof(Position));
					// memcpy(tempPosition, &gamePosition, sizeof(Position));

					// maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, myFinalMove);

					// myMove = *myFinalMove;
					
					// //free(tempPosition);
					// printf("\t\tMAX SCORE %d\n", maxScore);

					//Move *tempMove = findMove(&gamePosition, 0/*initial depth*/);
					//myMove = *tempMove;
					//free(tempMove);

				}

				sendMove( &myMove, mySocket );			//send our move
				//printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				//doMove( &gamePosition, &myMove );		//play our move on our position
				//printPosition( &gamePosition );
				
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



Move initRandom(char myColor, Position *aPosition){
	/**********************************************************/
// random player - not the most efficient implementation
	int i, j, k;
	int jumpPossible;
	int playerDirection;

	LinkedList *moves = moveFinder(aPosition);

	printAvailableMoves(moves);

	//deleteList(moves);



	if( myColor == WHITE )		// find movement's direction
		playerDirection = 1;
	else
		playerDirection = -1;

	jumpPossible = FALSE;		// determine if we have a jump available
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++ )
		{
			if( gamePosition.board[ i ][ j ] == myColor )
			{
				if( canJump( i, j, myColor, &gamePosition ) )
					jumpPossible = TRUE;
			}
		}
	}

	while( 1 )
	{
		i = rand() % (BOARD_ROWS);
		j = rand() % BOARD_COLUMNS;

		if( gamePosition.board[ i ][ j ] == myColor )		//find a piece of ours
		{

			myMove.tile[ 0 ][ 0 ] = i;		//piece we are going to move
			myMove.tile[ 1 ][ 0 ] = j;

			if( jumpPossible == FALSE )
			{
				myMove.tile[ 0 ][ 1 ] = i + 1 * playerDirection;
				myMove.tile[ 0 ][ 2 ] = -1;
				if( rand() % 2 == 0 )	//with 50% chance try left and then right
				{
					myMove.tile[ 1 ][ 1 ] = j - 1;
					if( isLegal( &gamePosition, &myMove ))
						break;

					myMove.tile[ 1 ][ 1 ] = j + 1;
					if( isLegal( &gamePosition, &myMove ))
						break;
				}
				else	//the other 50%...try right first and then left
				{
					myMove.tile[ 1 ][ 1 ] = j + 1;
					if( isLegal( &gamePosition, &myMove ))
						break;

					myMove.tile[ 1 ][ 1 ] = j - 1;
					if( isLegal( &gamePosition, &myMove ))
						break;
				}
			}
			else	//jump possible
			{
				if( canJump( i, j, myColor, &gamePosition ) )
				{
					k = 1;
					while( canJump( i, j, myColor, &gamePosition ) != 0 )
					{
						myMove.tile[ 0 ][ k ] = i + 2 * playerDirection;
						if( rand() % 2 == 0 )	//50% chance
						{

							if( canJump( i, j, myColor, &gamePosition ) % 2 == 1 )		//left jump possible
								myMove.tile[ 1 ][ k ] = j - 2;
							else
								myMove.tile[ 1 ][ k ] = j + 2;

						}
						else	//50%
						{

							if( canJump( i, j, myColor, &gamePosition ) > 1 )		//right jump possible
								myMove.tile[ 1 ][ k ] = j + 2;
							else
								myMove.tile[ 1 ][ k ] = j - 2;

						}

						if( k + 1 == MAXIMUM_MOVE_SIZE )	//maximum tiles reached
							break;

						myMove.tile[ 0 ][ k + 1 ] = -1;		//maximum tiles not reached

						i = myMove.tile[ 0 ][ k ];		//we will try to jump from this point in the next loop
						j = myMove.tile[ 1 ][ k ];


						k++;
					}
					break;
				}
			}
		}

	}

	return myMove;
// end of random
/**********************************************************/
}


LinkedList* moveFinder(Position *gamePosition){

	int i, j, moveDirection, canJumpOver = 0;

	LinkedList *mylinkedlist = (LinkedList*)malloc(sizeof(LinkedList));

	mylinkedlist = LinkedListInitializer(mylinkedlist);



	//Start iterating through the board to track all available moves
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++ )
		{
			if(gamePosition->board[i][j] != gamePosition->turn) continue;

			Move *tempMoveLeft = NULL, *tempMoveRight = NULL, *jumpMove = NULL;	

			if( canJump( i, j, gamePosition->turn, gamePosition ) ){
				//printf("JUMP POSSIBLE\n");

				jumpMove = (Move *)malloc(sizeof(Move));
				jumpMove->color = gamePosition->turn;

				if(!canJumpOver && mylinkedlist != NULL) deleteList(mylinkedlist); //Assignment mentions that jump move have priority over simple. So we don't need simple in that case

				multipleJumps(mylinkedlist, jumpMove, gamePosition, i, j, 0); 
				canJumpOver = 1;
				//continue;
			}
	

			if(!canJumpOver){	
				
				/*
				We create space for right and left moves otherwise it overwites.Remember here we simply
				want to track all the available moves in each position
				*/
				tempMoveLeft = (Move *)malloc(sizeof(Move));

				
				tempMoveLeft->color = gamePosition->turn;
				tempMoveLeft->tile[0][0] = i;		//piece we are going to move
				tempMoveLeft->tile[1][0] = j;
				tempMoveLeft->tile[0][1] = i + 1 * (gamePosition->turn == WHITE ? 1 : -1);
				tempMoveLeft->tile[0][2] = -1;
				tempMoveLeft->tile[1][1] = j - 1;

				//printf("INIT PLACE %d %d\n", tempMoveLeft->tile[0][0], tempMoveLeft->tile[1][0]);

				if(isLegal(gamePosition, tempMoveLeft)){
					//printf("NEW MOVE %d %d\n", tempMoveLeft->tile[0][1], tempMoveLeft->tile[1][1]);
					addElement(mylinkedlist, tempMoveLeft);}
				else
					free(tempMoveLeft);

				tempMoveRight = (Move *)malloc(sizeof(Move));
				tempMoveRight->color = gamePosition->turn;
				tempMoveRight->tile[0][0] = i;		//piece we are going to move
				tempMoveRight->tile[1][0] = j;
				tempMoveRight->tile[0][1] = i + 1 * (gamePosition->turn == WHITE ? 1 : -1);
				tempMoveRight->tile[0][2] = -1;
				tempMoveRight->tile[1][1] = j + 1;

				if(isLegal(gamePosition, tempMoveRight)){
					//printf("NEW MOVE %d %d\n", tempMoveRight->tile[0][1], tempMoveRight->tile[1][1]);
					addElement(mylinkedlist, tempMoveRight);}
				else
					free(tempMoveRight);

				
			}

		}

	}

	//Todo check if we need case when there is no move(I imagine not)

	return mylinkedlist;
}


void multipleJumps(LinkedList* mylist, Move* jumpMove,Position *gamePosition,int i, int j,int k ){
	
	//assert(MAXIMUM_MOVE_SIZE > k);

	printf("Pos JUMP (%d, %d)\n", i,j);
	int possibleJumps, playerDirection;

	possibleJumps = canJump(i, j, gamePosition->turn, gamePosition);

	if(possibleJumps){

		jumpMove->tile[0][k] = i;
		jumpMove->tile[1][k] = j;

		playerDirection = gamePosition->turn == WHITE ? 1 : -1;


		if(possibleJumps == 1) //can jump left
		{
			multipleJumps(mylist, jumpMove, gamePosition, i + 2*playerDirection, j-2, k+1);
			return;
		}else if(possibleJumps == 2){
			multipleJumps(mylist, jumpMove,  gamePosition, i + 2*playerDirection, j+2, k+1);
			return;
		}else{
			//IMPORTANT: DONT ASSIGN RIGHT AWAY POINTERS, CAUSES SEGMENTATION FAULT. Use memmove instead
			Move * altJumpMove = (Move *)malloc(sizeof(Move)); //create a duplicate move to follow the two different paths
			//altJumpMove = jumpMove;
			memmove(altJumpMove, jumpMove, sizeof(Move));

			multipleJumps(mylist, altJumpMove, gamePosition , i + 2 * playerDirection, j+2, k+1);
			multipleJumps(mylist, jumpMove, gamePosition, i + 2 * playerDirection, j-2, k+1);
			
			return;
		}

	}


	//To reach here no other jumps must be available. Check if the move is legit and add it to our list or free otherwise
	if( k + 1 != MAXIMUM_MOVE_SIZE ){
		jumpMove->tile[0][k+1] = -1;
	}
	

	if(isLegal(gamePosition, jumpMove))
		addElement(mylist, jumpMove);
	else
		free(jumpMove);

}


int evaluationFunction (Position *aPosition) {
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
	
	//check transposition table
	// Transp* curTransp= NULL;
	// if(((curTransp=retrieveTransposition(aPosition)) != NULL)&&(finalMove == NULL)){ //transposition found
		
		
	// 	//printf("Upperdepth: %d, lowerDepth: %d, currentDepth: %d\n", curTransp->upperDepth, curTransp->lowerDepth, depth);
	// 	if((curTransp->validity & 0x2)&&(curTransp->lowerBound >= beta)){
	// 		if(curTransp->lowerDepth >= depth){
	// 			hitsLower++;
	// 			return curTransp->lowerBound;
	// 		}
	// 	}

	// 	if((curTransp->validity & 0x2))
	// 		if(curTransp->lowerDepth >= depth)
	// 			alpha = max(alpha, curTransp->lowerBound);


			
	// 	if((curTransp->validity & 0x4)&&(curTransp->upperBound <= alpha)){
	// 		if(curTransp->upperDepth >= depth){
	// 			hitsUpper++;
	// 			return curTransp->upperBound;
	// 		}
	// 	}




	// 	if((curTransp->validity & 0x4))
	// 		if(curTransp->upperDepth >= depth)
	// 			beta = min (beta, curTransp->upperBound);
	//}
	

	if (depth <= 0){ //if we reached the maximum depth of our recursion
		//if(!quiescenceSearch(aPosition)){ // and there are no captures we can see
			return evaluationFunction(aPosition); //return heuristic
		//}
		
	}

	LinkedList *moveList = moveFinder(aPosition);   //finding all legal moves in this position
	Move * tempData = NULL;


	if (moveList == NULL || moveList->head->data == NULL){     //If for any reason, no more moves are available
		//deleteList(moveList);
		return evaluationFunction(aPosition);
	}

	Position* tempPosition = malloc(sizeof(Position));
	//Move *tempMove = malloc(sizeof(Move));
	int tempScore, g;

	int a, b;


	if (maximizingPlayer){

		g = -INFINITY;
		a = alpha;
		while((g<beta)&&((tempData = removeFirst(moveList)) != NULL)){ //for each child position

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[1][0], tempData->tile[1][1]);
			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);

			//printf("MAXIMIZER Searching Move on depth %d:\n", depth);

			tempScore = alpha_beta(tempPosition, depth-1, a, beta, 0, NULL);

			//g = max(g, tempScore);
			

			if(g < tempScore){
				g = tempScore;
				if(finalMove != NULL){
					//printf("MAXIMIZER %d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
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
		while((g>alpha)&&(tempData = removeFirst(moveList)) != NULL){ //for each child position

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[1][0], tempData->tile[1][1]);

			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);
			//printf("MINIMIZER Searching Move on depth %d:\n", depth);
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

	//deleteList(moveList);
	//free(tempPosition);
	
	// if(g <= alpha)
	// 	saveUpperTransposition(aPosition, g, depth);
	// if(g > alpha && g < beta)
	// 	saveTransposition(aPosition, g, g, depth);
	// if (g>= beta)
	// 	saveLowerTransposition(aPosition, g, depth);
	

	return g;
}