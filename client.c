#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "client.h"

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
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

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will send opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{
					// short choice = 1;

					// switch(choice){
					// 	case 1: 
							myMove = initRandom(myColor,&gamePosition);
						// 	printf("Random Choice 1\n");
						// 	break;
						// default:
						// 	myMove = initRandom(myColor);
						// 	printf("Default choice\n");
						// 	break;
					// }
					

					//Move *tempMove = findMove(&gamePosition, 0/*initial depth*/);
					//myMove = *tempMove;
					//free(tempMove);

				}

				sendMove( &myMove, mySocket );			//send our move
				printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );
				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
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

	int i, j, moveDirection, jumpPossible = 0;

	LinkedList *mylinkedlist = (LinkedList*)malloc(sizeof(LinkedList));

	LinkedListInitializer(mylinkedlist);

	if( gamePosition->turn == WHITE )		// find movement's direction
		moveDirection = 1;
	else
		moveDirection = -1;


	//Start iterating through the board to track all available moves
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++ )
		{
			if(gamePosition->board[i][j] != gamePosition->turn) continue;



			// if( canJump( i, j, curColor, aPosition ) ){
			// 	if(!jumpPossible)
			// 		emptyList(moveList); //any simple moves are deleted
			// 	move = malloc(sizeof(Move));
			// 	move->color = curColor;
			// 	recursiveFollowJump(moveList, move, 0, i, j, aPosition); //FOUND! IF MORE THAN ONE JUMP POSSIBLE THEN PUSHING THE SAME MALLOC'd move
			// 	jumpPossible = TRUE;
			// }

			Move *tempMoveLeft;
			Move *tempMoveRight;

			//moveDirection = canJump(i, j, gamePosition->turn, gamePosition);
			//TODO first work on simple case and then complicate things
			if(!jumpPossible){	//check what statement here is needed
				

				tempMoveLeft = malloc(sizeof(Move));
				tempMoveRight = malloc(sizeof(Move));
				tempMoveLeft->tile[ 0 ][ 0 ] = i;		//piece we are going to move
				tempMoveLeft->tile[ 1 ][ 0 ] = j;

				tempMoveLeft->tile[ 0 ][ 1 ] = i + 1 * moveDirection;
				tempMoveLeft->tile[ 0 ][ 2 ] = -1;

				tempMoveLeft->tile[ 1 ][ 1 ] = j - 1;
				if(isLegal(gamePosition, tempMoveLeft)){
					printf("NEW MOVE %d %d\n", tempMoveLeft->tile[0][ 1 ], tempMoveLeft->tile[1][1] = j-1);
					addElement(mylinkedlist, tempMoveLeft);}
				else
					free(tempMoveLeft);

				tempMoveRight->tile[ 0 ][ 0 ] = i;		//piece we are going to move
				tempMoveRight->tile[ 1 ][ 0 ] = j;

				tempMoveRight->tile[ 0 ][ 1 ] = i + 1 * moveDirection;
				tempMoveRight->tile[ 0 ][ 2 ] = -1;

				tempMoveRight->tile[ 1 ][ 1 ] = j + 1;
				if(isLegal(gamePosition, tempMoveRight)){
					printf("NEW MOVE %d %d\n", tempMoveRight->tile[0][ 1 ], tempMoveRight->tile[1][1] = j-1);
					addElement(mylinkedlist, tempMoveRight);}
				else
					free(tempMoveRight);

				
			}

		}

	}

	//Todo add case no move

	return mylinkedlist;
}


// void alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove){  //recursive minimax function.
	
// 	//check transposition table
// 	Transp* curTransp= NULL;
// 	if(((curTransp=retrieveTransposition(aPosition)) != NULL)&&(finalMove == NULL)){ //transposition found
		
		
// 		//printf("Upperdepth: %d, lowerDepth: %d, currentDepth: %d\n", curTransp->upperDepth, curTransp->lowerDepth, depth);
// 		if((curTransp->validity & 0x2)&&(curTransp->lowerBound >= beta)){
// 			if(curTransp->lowerDepth >= depth){
// 				hitsLower++;
// 				return curTransp->lowerBound;
// 			}
// 		}

// 		if((curTransp->validity & 0x2))
// 			if(curTransp->lowerDepth >= depth)
// 				alpha = max(alpha, curTransp->lowerBound);


			
// 		if((curTransp->validity & 0x4)&&(curTransp->upperBound <= alpha)){
// 			if(curTransp->upperDepth >= depth){
// 				hitsUpper++;
// 				return curTransp->upperBound;
// 			}
// 		}




// 		if((curTransp->validity & 0x4))
// 			if(curTransp->upperDepth >= depth)
// 				beta = min (beta, curTransp->upperBound);
// 	}
	

// 	if (depth <= 0){ //if we reached the maximum depth of our recursion
// 		if(!quiescenceSearch(aPosition)){ // and there are no captures we can see
// 			return evalPosition(aPosition); //return heuristic
// 		}
		
// 	}

// 	list *moveList = findAllMoves(aPosition);   //finding all legal moves in this position
// 	Move * tempData = NULL;


// 	if (top(moveList) == NULL){     //If for any reason, no more moves are available
// 		freeList(moveList);
// 		return evalPosition(aPosition);
// 	}

// 	Position* tempPosition = malloc(sizeof(Position));
// 	//Move *tempMove = malloc(sizeof(Move));
// 	int tempScore, g;

// 	int a, b;


// 	if (maximizingPlayer){

// 		g = -INFINITY;
// 		a = alpha;
// 		while((g<beta)&&((tempData = pop(moveList)) != NULL)){ //for each child position


// 			memcpy(tempPosition, aPosition, sizeof(Position));
// 			doMove(tempPosition, tempData);

// 			tempScore = alpha_beta(tempPosition, depth-1, a, beta, 0, NULL);

// 			//g = max(g, tempScore);
			

// 			if(g < tempScore){
// 				g = tempScore;
// 				if(finalMove != NULL){
// 					//printf("%d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
// 					memcpy(finalMove, tempData, sizeof(Move));
// 				}
// 			}
// 			a = max(a, g);
// 			//alpha = max(alpha, tempScore);
// 			//if( beta <= alpha){ free(tempData); break;}
// 			free(tempData);
// 		}/*
// 		freeList(moveList);
// 		free(tempPosition);
// 		saveTransposition(aPosition, alpha, depth);
// 		//printf("alpha: %d\n", alpha);
// 		return alpha;*/
// 	}else{
// 		g = INFINITY;
// 		b = beta;
// 		while((g>alpha)&&(tempData = pop(moveList)) != NULL){ //for each child position

// 			memcpy(tempPosition, aPosition, sizeof(Position));
// 			doMove(tempPosition, tempData);
// 			//printf("Searching Move on depth %d:\n", depth);
// 			//printMove(tempData);
// 			tempScore = alpha_beta(tempPosition, depth-1, alpha, b, 1, NULL);
			
// 			g = min(g, tempScore);
// 			b = min(b, g);

// 			//if( beta <= alpha){ free(tempData); break;}
// 			free(tempData);
// 		}
// 		/*
// 		saveTransposition(aPosition, beta, depth);
// 		//printf("%d\n", beta);
// 		return beta;
// 		*/
// 	}

// 	freeList(moveList);
// 	free(tempPosition);
	
// 	// if(g <= alpha)
// 	// 	saveUpperTransposition(aPosition, g, depth);
// 	// if(g > alpha && g < beta)
// 	// 	saveTransposition(aPosition, g, g, depth);
// 	// if (g>= beta)
// 	// 	saveLowerTransposition(aPosition, g, depth);
	

// 	return g;
// }


