#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
// #include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "client.h"
#include <sys/time.h>

#define MAX_DEPTH 2
#define INFINITY 999999999

// #define MAX_TIME 7
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

					// int minimax(Position *gamePosition, int depth, int ismaximizingPlayer, Move* finalMove){
					maxScore = minimax(tempPosition, MAX_DEPTH, TRUE, &myMove,1);

					//maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, &myMove);

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


void multipleJumps(LinkedList* moveList, Move* move, int k /* depth of recursion*/,int i, int j, Position *aPosition){
	
	int possibleJumps, playerDirection;

	move->tile[0][k] = i;
	move->tile[1][k] = j;

	possibleJumps = canJump(i, j, move->color, aPosition);


	playerDirection = aPosition->turn == WHITE ? 1 : -1;

	if(possibleJumps){
		if(possibleJumps == 1) //can jump left
		{
			multipleJumps(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
			return;
		}else if(possibleJumps == 2) //can jump right
		{
			multipleJumps(moveList, move, k+1, i + 2*playerDirection, j+2, aPosition);
			return;
		}else if(possibleJumps == 3) //we need to split the jumps
		{
			//copying move:
			Move * newMove = malloc(sizeof(Move));
			memcpy(newMove, move, sizeof(Move));

			//following both left and right
			multipleJumps(moveList, move, k+1, i + 2*playerDirection, j-2, aPosition);
			multipleJumps(moveList, newMove, k+1, i + 2*playerDirection, j+2, aPosition);
			return;

		}
	}


	if( k + 1 != MAXIMUM_MOVE_SIZE ){
		move->tile[0][k+1] = -1;
	}


	if(isLegal(aPosition, move)){
		addElement(moveList, move);
		}
	else
		free(move);
	return;
	
}

void simpleMove(LinkedList* moveList, Position *gamePos, int i, int j, int playerDirection, int moveDirection){
	Move *move = (Move*)malloc(sizeof(Move));
	move->color = gamePos->turn;
	move->tile[0][0] = i;
	move->tile[1][0] = j;
	move->tile[0][1] = i + playerDirection;
	move->tile[1][1] = j + moveDirection;
	move->tile[0][2] = -1;

	if(isLegal(gamePos, move)){
		addElement(moveList, move);}
	else
		free(move);
}

LinkedList* moveFinder(Position *gamePos) {
	//todo change variables here
	int i, j, jumpPossible = 0, movePossible = 0, playerDirection;
	LinkedList* moveList = (LinkedList*)malloc(sizeof(LinkedList));

	//Jump a simple initiallization
	moveList = LinkedListInitializer(moveList);
	Move *move;

	playerDirection = gamePos->turn == WHITE ? 1 : -1; 


	//Start iterating through the board to track all available moves
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++)
			{
				if( gamePos->board[ i ][ j ] != gamePos->turn ) continue;
					
					//From assignment we give priority to jumps than simple moves
					if( canJump( i, j, gamePos->turn, gamePos ) ){
						//printf("JUMP POSSIBLE\n");
						if(!jumpPossible) deleteList(moveList); //any simple moves are deleted in case we find jump moves
						
						move = malloc(sizeof(Move));
						memset(move,0,sizeof(Move));	//Valgrid stop shouting
						move->color = gamePos->turn;
						multipleJumps(moveList, move, 0, i, j, gamePos); 
						jumpPossible = 1;
					}else if((jumpPossible == 0)){ 

						//Check both for left and right jump. In case we find a valid jump move add it to our list
						simpleMove(moveList,gamePos,i,j, playerDirection,-1);

						simpleMove(moveList,gamePos,i,j, playerDirection,1);
						// move = (Move*)malloc(sizeof(Move));
						// move->color = gamePos->turn;
						// move->tile[0][0] = i;
						// move->tile[1][0] = j;
						// move->tile[0][1] = i + playerDirection;
						// move->tile[1][1] = j-1;
						// move->tile[0][2] = -1;

						// if(isLegal(gamePos, move)){
						// 	addElement(moveList, move);}
						// else
						// 	free(move);

						// move = (Move*) malloc(sizeof(Move));
						// move->color = gamePos->turn;
						// move->tile[0][0] = i;
						// move->tile[1][0] = j;
						// move->tile[0][1] = i + playerDirection;
						// move->tile[1][1] = j+1;
						// move->tile[0][2] = -1;

						// if(isLegal(gamePos, move)){
						// 	addElement(moveList, move);}
						// else
						// 	free(move);

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

// //TODO will change that eventually
// int evaluationFunction (Position *aPosition) {
//     int i,j, evaluation = 0;
    
//     for (i = 0; i < BOARD_ROWS; i++)
//     {
//         for ( j = 0; j < BOARD_COLUMNS; j++)
//         {
//             if (aPosition->board[i][j] == myColor){
//                evaluation += 100;
//                //printf("For piece in %d %d, of color: %d, we add...\n", i, j, myColor);
//                if(myColor == WHITE)
//                	evaluation += i*1;
//                else
//                	evaluation += (BOARD_ROWS-i-1)*1;


//            	}
//             else if (aPosition->board[i][j] == getOtherSide(myColor)){
//                evaluation -= 100;
//                //printf("For piece in %d %d, of color: %d, we sub...\n", i, j, getOtherSide(myColor));
//                if(myColor == BLACK)
//                	evaluation -= i*1;
//                else
//                	evaluation -= (BOARD_ROWS-i-1)*1;
//            	}

//         }
//     }
//     evaluation = evaluation + aPosition->score[myColor]*90 - aPosition->score[getOtherSide(myColor)]*90;

//     return evaluation;
// }


//TODO will change that eventually
int evaluationFunction (Position *aPosition) {
    int i,j, evaluation = 0;
    
    for (i = 0; i < BOARD_ROWS; i++)
    {
        for ( j = 0; j < BOARD_COLUMNS; j++)
        {
            if (aPosition->board[i][j] == myColor){
               evaluation += 100;
               //printf("For piece in %d %d, of color: %d, we add...\n", i, j, myColor);
               // if(myColor == WHITE)
               // 	evaluation += i*1;
               // else
               // 	evaluation += (BOARD_ROWS-i-1)*1;


           	}
            else if (aPosition->board[i][j] == getOtherSide(myColor)){
               evaluation -= 100;
               //printf("For piece in %d %d, of color: %d, we sub...\n", i, j, getOtherSide(myColor));
               // if(myColor == BLACK)
               // 	evaluation -= i*1;
               // else
               // 	evaluation -= (BOARD_ROWS-i-1)*1;
           	}

        }
    }
    evaluation = evaluation + aPosition->score[myColor]*90 - aPosition->score[getOtherSide(myColor)]*90;

    return evaluation;
}

int max(int num1, int num2){
	return (num1 > num2) ? num1 : num2;
} 


int min(int num1, int num2){
	return (num1 < num2) ? num1 : num2;
} 



//https://www.javatpoint.com/ai-alpha-beta-pruning
int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove){  //recursive minimax function.
	

	if (depth <= 0){ //if we reached the maximum depth of our recursion and there are no captures we can see
			return evaluationFunction(aPosition); //return heuristic
	}

	LinkedList *moveList = moveFinder(aPosition);   //finding all legal moves in this position
	Move *tempData = NULL;


	if (moveList == NULL || moveList->head == NULL){     //If for any reason, no more moves are available
		deleteList(moveList);
		return evaluationFunction(aPosition);
	}

	Position* tempPosition = malloc(sizeof(Position));
	int tempScore, g;

	int a, b;


	if (maximizingPlayer){

		g = -INFINITY;
		a = alpha;
		while((g<beta)&&((tempData = removeFirst(moveList)) != NULL)){ //for each child position
			printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);


			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);

			tempScore = alpha_beta(tempPosition, depth-1, a, beta, 0, NULL);

			//g = max(g, tempScore);
			

			if(g < tempScore){
				g = tempScore;
				if(finalMove != NULL){
					//printf("MAXIMIZER %d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
					printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

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
		while((g>alpha)&&(tempData = removeFirst(moveList)) != NULL){ //for each child position

			printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

			memcpy(tempPosition, aPosition, sizeof(Position));
			doMove(tempPosition, tempData);
			//printf("MINIMIZER Searching Move on depth %d:\n", depth);
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

	deleteList(moveList);
	free(tempPosition);
	return g;
}


//Based on pseudocode and ideas from those articles
//https://en.wikipedia.org/wiki/Minimax
//https://www.researchgate.net/figure/MiniMax-Algorithm-Pseduo-Code-In-Fig-3-there-is-a-pseudo-code-for-NegaMax-algorithm_fig2_262672371
int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot){

	//In case we reach the depth we want return evaluation
	if(depth == 0){	//TODO or node is terminal node
		return evaluationFunction(gamePos);
	}


	LinkedList *allMoves = moveFinder(gamePos);
	Move* childData = NULL;

	if (allMoves == NULL || allMoves->head == NULL){     //If for any reason, no more moves are available
		deleteList(allMoves);
		return evaluationFunction(gamePos);
	}

	Position* tempPosition = malloc(sizeof(Position));

	int value,tempScore;

	if(ismaximizingPlayer){

		value = -INFINITY;
		while((childData = removeFirst(allMoves)) != NULL){
			printf("MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);
			
			memcpy(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, childData);

			tempScore = max(value, minimax(tempPosition, depth - 1, FALSE, NULL,0));
			//printf("\tDepth %d and val %d\n", depth, value);

			if(tempScore > value){
				value = tempScore;
				if(isRoot){
					printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);

					memcpy(finalMove, childData, sizeof(Move));
				}
			}
			free(childData);
		}
		return value;
	}else{
		value = INFINITY;
		while((childData = removeFirst(allMoves)) != NULL){
			printf("MIN \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);
			//printf("depth %d and val %d\n", depth, value);
			memcpy(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, childData);

			value = min(value, minimax(tempPosition, depth - 1, TRUE, NULL,0));
			//printf("\tDepth %d and val %d\n", depth, value);

			free(childData);
		}
		return value;
	}

	deleteList(allMoves);
	free(tempPosition);
}