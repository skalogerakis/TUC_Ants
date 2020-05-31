#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "client.h"
#include <sys/time.h>
#include <limits.h>

#define MAX_DEPTH 10

//USHRT_MAX has value 65535. It seems highly unlikely that we will encouter with a larger value
#define INFINITY USHRT_MAX

// #define MAX_TIME 3
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

//TODO mess with bit improvements

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
					memmove(tempPosition, &gamePosition, sizeof(Position));

					//myMove = initRandom(myColor);

					//maxScore = minimax(tempPosition, MAX_DEPTH, TRUE, &myMove,1);

					// maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, &myMove,1 );
					// maxScore = alpha_beta1(tempPosition, MAX_DEPTH, -INFINITY, INFINITY, 1, &myMove);
					//maxScore = iterativeDeepening(tempPosition, &myMove);
					// MTDF(Position* aPosition, int f, char d, Move* finalMove)
					// maxScore = MTDFSearch(tempPosition, evaluationFunction(tempPosition) ,MAX_DEPTH, &myMove);

					// int NegaScout(Position *gamePos, char depth, int alpha, int beta, Move* finalMove, int isRoot)
					maxScore = NegaScout(tempPosition,MAX_DEPTH,-INFINITY, INFINITY, 1, &myMove);					

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

/*
	Jump cases using this function.
*/
void multipleJumps(LinkedList* moveList, Move* move, short k /* depth of recursion*/,short i, short j, Position *gamePos){
	
	//short jumpDir, playerDir;
	

	move->tile[0][k] = i;
	move->tile[1][k] = j;

	// jumpDir = canJump(i, j, move->color, gamePos);
	MoveUtil.jumpDirection = canJump(i, j, move->color, gamePos);

	MoveUtil.playDirection = gamePos->turn == WHITE ? 1 : -1;

	//Find all possible jumps recursively until no other jumps are available
	switch(MoveUtil.jumpDirection)
	{
		case 1:
			multipleJumps(moveList, move, k+1, i + 2*MoveUtil.playDirection, j-2, gamePos);
			return;
		case 2:
			multipleJumps(moveList, move, k+1, i + 2*MoveUtil.playDirection, j+2, gamePos);
			return;
		case 3:	//Both left and right jumps are available, so duplicate move and move on both ways
			{
				Move *newMove = malloc(sizeof(Move));
				memmove(newMove, move, sizeof(Move));

				//following both left and right jump
				multipleJumps(moveList, move, k+1, i + 2*MoveUtil.playDirection, j-2, gamePos);
				multipleJumps(moveList, newMove, k+1, i + 2*MoveUtil.playDirection, j+2, gamePos);
				return;
			}
		default:
			//When we reach here we are out of jumps so 
			if( k + 1 != MAXIMUM_MOVE_SIZE ){
				move->tile[0][k+1] = -1;
			}

			moveLegality(moveList, move, gamePos);
			return;
	}

	return;
	
}

/*
	Check if move is legal and adds them in our list if true
*/
void moveLegality(LinkedList* moveList, Move* move, Position* gamePos){
	if(isLegal(gamePos, move))
		addElement(moveList, move);
	else
		free(move);

	return;
}

/*
	Function responsible to handle simple moves(both left and right)
*/
void simpleMove(LinkedList* moveList, Position *gamePos, short i, short j, short playerDirection, short moveDirection){
	Move *move = (Move*)malloc(sizeof(Move));
	move->color = gamePos->turn;
	move->tile[0][0] = i;
	move->tile[1][0] = j;
	move->tile[0][1] = i + playerDirection;
	move->tile[1][1] = j + moveDirection;
	move->tile[0][2] = -1;

	//In case illegal move free 
	moveLegality(moveList, move, gamePos);
	return;
}

/*
	Function responsible to track all available moves(Simple moves and jumps)
*/
LinkedList* moveFinder(Position *gamePos) {
	//todo change variables here
	short i, j, jumpPossible = 0, playerDirection;
	LinkedList* moveList = (LinkedList*)malloc(sizeof(LinkedList));

	//Jump a simple initiallization
	moveList = LinkedListInitializer(moveList);
	Move *move;

	playerDirection = gamePos->turn == WHITE ? 1 : -1; 


	//Start iterating through the board to track all available moves
	//UPDATED: Loop unrolling works fine
	for( i = 0; i < BOARD_ROWS; i+=4 )
	{
		for( j = 0; j < BOARD_COLUMNS; j+=4)
			{
				
				jumpPossible = moveIterator(moveList, gamePos, move, i , j, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+1 , j, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+2 , j, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+3 , j, playerDirection,jumpPossible);

				jumpPossible = moveIterator(moveList, gamePos, move, i , j+1, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+1 , j+1, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+2 , j+1, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+3 , j+1, playerDirection,jumpPossible);

				jumpPossible = moveIterator(moveList, gamePos, move, i , j+2, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+1 , j+2, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+2 , j+2, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+3 , j+2, playerDirection,jumpPossible);

				jumpPossible = moveIterator(moveList, gamePos, move, i , j+3, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+1 , j+3, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+2 , j+3, playerDirection,jumpPossible);
				jumpPossible = moveIterator(moveList, gamePos, move, i+3 , j+3, playerDirection,jumpPossible);
				
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

short moveIterator(LinkedList* moveList,Position* gamePos, Move* move ,short i, short j, short playerDirection, short jumpPossible){
	if( gamePos->board[ i ][ j ] != gamePos->turn ) return jumpPossible;
					
	//From assignment we give priority to jumps than simple moves
	if( canJump( i, j, gamePos->turn, gamePos ) ){
		//printf("JUMP POSSIBLE\n");
		if(!jumpPossible) deleteList(moveList); //any simple moves are deleted in case we find jump moves
		
		//In contrary with simple move we create move before and we pass as parameter. Everything needs to store in the same move
		move = malloc(sizeof(Move));
		memset(move,0,sizeof(Move));	//Valgrid stop shouting
		move->color = gamePos->turn;
		multipleJumps(moveList, move, 0, i, j, gamePos); 
		jumpPossible = 1;
	}else if((jumpPossible == 0)){ 

		//Check both for left and right jump. In case we find a valid jump move add it to our list
		simpleMove(moveList,gamePos,i,j, playerDirection,-1);

		simpleMove(moveList,gamePos,i,j, playerDirection,1);
	
	}
	return jumpPossible;
}


//TODO will change that eventually
// int evaluationFunction (Position *aPosition) {
//     int i,j, evaluation = 0;
    
//     for (i = 0; i < BOARD_ROWS; i++)
//     {
//         for ( j = 0; j < BOARD_COLUMNS; j++)
//         {
//             if (aPosition->board[i][j] == myColor){
//                evaluation += 100;
//                //printf("For piece in %d %d, of color: %d, we add...\n", i, j, myColor);
//                // if(myColor == WHITE)
//                // 	evaluation += i*1;
//                // else
//                // 	evaluation += (BOARD_ROWS-i-1)*1;


//            	}
//             else if (aPosition->board[i][j] == getOtherSide(myColor)){
//                evaluation -= 100;
//                //printf("For piece in %d %d, of color: %d, we sub...\n", i, j, getOtherSide(myColor));
//                // if(myColor == BLACK)
//                // 	evaluation -= i*1;
//                // else
//                // 	evaluation -= (BOARD_ROWS-i-1)*1;
//            	}

//         }
//     }
//     evaluation = evaluation + aPosition->score[myColor]*90 - aPosition->score[getOtherSide(myColor)]*90;

//     return evaluation;
// }

int evaluationFunction(Position *gamePos){
	int myCounterPieces=0,opponentCounterPieces=0,myKings=0,opponentKings=0,myCornerCounterPieces=0,opponentCornerCounterPieces=0;
	int i=0,j=0;
	int abs1=0,abs2=0;
	int evaluation=0;
	for(i = 0; i < BOARD_SIZE; i++ ){
		for(j = 0; j < BOARD_SIZE; j++ ){
			if(myColor==WHITE){
				if( ( gamePos->board[ i ][ j ] == myColor ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					myCornerCounterPieces++;
				}
				else if( ( gamePos->board[ i ][ j ] == getOtherSide(myColor) ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					opponentCornerCounterPieces++;
				}
				else if(( gamePos->board[ i ][ j ] == myColor )){
					myCounterPieces++;
				}
				else if(( gamePos->board[ i ][ j ] == getOtherSide(myColor) )){
					opponentCounterPieces++;
				}
			}
			else if(myColor==BLACK){
				if( ( gamePos->board[ i ][ j ] == myColor ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					myCornerCounterPieces++;
				}
				else if( ( gamePos->board[ i ][ j ] == getOtherSide(myColor) ) && ( j!=BOARD_SIZE-1 ) && (j!=0) ){
					opponentCornerCounterPieces++;
				}
				else if(( gamePos->board[ i ][ j ] == myColor )){
					myCounterPieces++;
				}
				else if(( gamePos->board[ i ][ j ] == getOtherSide(myColor) )){
					opponentCounterPieces++;
					
				}
			}
		}
	}
	abs1=myCounterPieces*100+myCornerCounterPieces*120;
	abs2=opponentCounterPieces*100+opponentCornerCounterPieces*120;	
	evaluation=abs1-abs2 + ( gamePosition.score[ myColor ] * 140) - ( gamePosition.score[ getOtherSide( myColor ) ] * 140);

	short myPiecesCount=0,opponentPiecesCount;
	for(i = 0; i < BOARD_SIZE; i++ ){
		for(j = 0; j < BOARD_SIZE; j++ ){
			if(gamePos->board[i][j] == myColor){
				myPiecesCount++;
			}
			else if(gamePos->board[i][j] == getOtherSide(myColor)){
				opponentPiecesCount++;
			}
		}
	}

	return evaluation;
}

int max(int num1, int num2){
	return (num1 > num2) ? num1 : num2;
} 


int min(int num1, int num2){
	return (num1 < num2) ? num1 : num2;
} 

/*
	Random implementation as give from assignment files
*/
Move initRandom(char myColor, Position *aPosition){

	int i, j, k;
	int jumpPossible;
	int playerDirection;

	LinkedList *moves = moveFinder(aPosition);

	printAvailableMoves(moves);

	removeFirst(moves);


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

}

// https://en.wikipedia.org/wiki/Quiescence_search
/*
	Classic implementation with only captures are taken into account
	 Check there are no hidden traps and to get a better estimate of its value. 
	 Quiescence search attempts to mitigate the horizon effect(a problem in artificial intelligence which can occur when all moves from a given node in a game tree are searched to a fixed depth) 
	 by extending the search depth in volatile positions where the heuristic value may have significant fluctuations between moves.
*/
short quiescenceSearch(Position* gamePos){
		short i, j, playerDirection;
		//quiescence search
		// determine if we have a jump available

		playerDirection = gamePos->turn == WHITE ? 1 : -1; 

		for( i = 0; i < BOARD_ROWS; i++ )
		{
			for( j = 0; j < BOARD_COLUMNS; j++ )
			{
				if( gamePos->board[ i ][ j ] == gamePos->turn )
				{
										
					// if( canJump( i, j, gamePos->turn, gamePos ) || (i + playerDirection == (BOARD_ROWS-1)) || (i + playerDirection == 0)){
					//Tried to make posistion next to finish not quiet and get a better position but could not handle it in time. TODO ADD IMPROVED EVALUATION IN THAT CASE
					if( canJump( i, j, gamePos->turn, gamePos )){
						return TRUE;
					}
				}
			}
		}
		return FALSE;
}

//TODO SEE IF WE WANT TO ADD
/*
	https://stackoverflow.com/questions/41756443/how-to-implement-iterative-deepening-with-alpha-beta-pruning
	TODO PASS TIME TO OUR ALPHA BETA IN CASE NO OTHER IMPLEMENTATion. TODO REVISIT THAT
	https://github.com/nealyoung/CS171/blob/master/AI.java
*/
// int iterativeDeepeningSearch(Position* gamePos, Move* finalMove)
// {
// 	int f = evaluationFunction(gamePos); //first guess TODO check that
// 	char d=1;
// 	// clock_t cstart = clock();
// 	/*
// 		Handle Time in C
// 		https://stackoverflow.com/questions/19084596/how-to-use-seconds-time-in-c-program-as-a-counter
// 	*/
// 	clock_t startTime;
// 	double timeElapsed;

// 	startTime = clock();
// 	//TODO change that as well	
// 	// while(1)
// 	// {

// 	// 	// f = MTDF(aPosition, f, d, finalMove);
// 	// 	f = alpha_beta(gamePos, d, -INFINITY, INFINITY, 1, finalMove, 1);

// 	// 	timeElapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;
//  //        if (timeElapsed >= MAX_TIME ||(d > MAX_DEPTH) ){
//  //        	printf("Time timeElapsed\n");
//  //           	printf("======================================================\n");
// 	// 		printf("Max Score: %d\n", f);
// 	// 		printf("Time used: %f\n", timeElapsed);
// 	// 		printf("Depth of iteration: %d\n", d);
// 	// 		printf("======================================================\n");
// 	// 		break;
//  //        }
        
// 	// 	d +=1;

// 	// }
// 	// return f;

// 	//NEW CHECK THAT
// 	//https://homepages.cwi.nl/~paulk/theses/Carolus.pdf
// 	for(d = 1; d <= MAX_DEPTH; d++){
// 		f = alpha_beta(gamePos, d, -INFINITY, INFINITY, 1, finalMove, 1);

// 		timeElapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;
//         if (timeElapsed >= MAX_TIME){
//         	printf("Time timeElapsed\n");
//            	printf("======================================================\n");
// 			printf("Max Score: %d\n", f);
// 			printf("Time used: %f\n", timeElapsed);
// 			printf("Depth of iteration: %d\n", d);
// 			printf("======================================================\n");
// 			break;
//         }
// 	}
// 	return f;
// }



/*
	Also read for report
	THIS USES WHAT WE WANT https://www.cs.unm.edu/~aaron/downloads/qian_search.pdf
	https://en.wikipedia.org/wiki/MTD-f
	https://www.chessprogramming.org/MTD(f)
	https://people.csail.mit.edu/plaat/mtdf.html ALSO TIME ITERATIVE DEEEPENING
*/


int MTDFSearch(Position* gamePos, int f, int d, Move* finalMove){
	int beta;
	//int g = f;
	//f = evaluationFunction(gamePos);
	int score = f;
	int upperBound = INFINITY;
	int lowerBound = - INFINITY;

	//Move* aMove = malloc(sizeof(Move));
	do
	{
		if (score == lowerBound)
			beta = score + 1;
		else
			beta = score;

		score = alpha_beta(gamePos, d, beta-1, beta, 1, finalMove,1);
		if (score < beta){
			upperBound = score;
			
		}else{ //keeping move
			//memcpy(finalMove, aMove, sizeof(Move));
			lowerBound = score;
		}
	}while(lowerBound < upperBound);
	// free(aMove);;
	return score;


}

/*
	https://www.cs.unm.edu/~aaron/downloads/qian_search.pdf
	https://www.chessprogramming.org/NegaScout

*/
int NegaScout(Position *gamePos, char depth, int alpha, int beta, int isRoot, Move* finalMove){
	
	int score,n;

	//No quiescense search here. Sources mention that Negascout might need to re-search so we cannot expand further
	if (depth <= 0){
		return evaluationFunction(gamePos);
	}

	score = -INFINITY;
	n = beta;

	LinkedList *moveList = moveFinder(gamePos);   //finding all legal moves in this position
	Move *tempData = NULL;


	if (moveList == NULL || moveList->head == NULL){     //If no more moves available
		deleteList(moveList);
		return evaluationFunction(gamePos);
	}

	Position* tempPosition = malloc(sizeof(Position));
	int cur;

	while((tempData = removeFirst(moveList)) != NULL){ //for each child position

		//printf("\t\tDepth %d, Alpha %d, Beta %d ,Start from (%d, %d) and go to (%d, %d) \n",depth, alpha, beta ,tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);
		memmove(tempPosition, gamePos, sizeof(Position));
		doMove(tempPosition, tempData);

		cur = -NegaScout(gamePos, depth-1,-n,-alpha,0,finalMove);

		if(cur > score){
			if(n == beta || depth <=2 ){
				if(isRoot){
					memmove(finalMove, tempData, sizeof(Move));
					//printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

				}
				score = cur;
			}else{
				score = -NegaScout(gamePos, depth-1,-beta,-cur,0,finalMove);
			}
		}

		if(score > alpha) alpha = score;

		if(alpha >= beta) return alpha;
		n = alpha + 1;
		

		// tempScore = alpha_beta(tempPosition, depth-1, alpha, beta, 0, finalMove,0);

		// if(value < tempScore){
		//  	value = tempScore;
		// 	if(isRoot){
		// 		//printf("MAXIMIZER %d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
		// 		//printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

		// 		memmove(finalMove, tempData, sizeof(Move));
		// 	}
		// }

		// alpha = max(alpha, value);
		// if( beta <= alpha){ 
		// 	free(tempData); 
		// 	break;
		// }
		// free(tempData);
	}
	free(tempData);
	return score;

}

//https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
//https://www.javatpoint.com/ai-alpha-beta-pruning
/*
	Alpha beta pruning implementation in similar fashion with minimax
*/
int alpha_beta(Position *gamePos, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove, int isRoot){  
	
	//In case we reached the maximum availanle depth and we are in quiet position return evalution 
	if (depth <= 0){
		if(!quiescenceSearch(gamePos)){	// In case we are not in a quiet position keep on with the recursion until we reach that point
			return evaluationFunction(gamePos); 
		}
	}

	LinkedList *moveList = moveFinder(gamePos);   //finding all legal moves in this position
	Move *tempData = NULL;


	if (moveList == NULL || moveList->head == NULL){     //If no more moves available
		deleteList(moveList);
		return evaluationFunction(gamePos);
	}

	Position* tempPosition = malloc(sizeof(Position));
	int tempScore, value;

	if (maximizingPlayer){

		value = -INFINITY;
		while((tempData = removeFirst(moveList)) != NULL){ //for each child position

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);
			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, tempData);

			tempScore = alpha_beta(tempPosition, depth-1, alpha, beta, 0, finalMove,0);

			if(value < tempScore){
			 	value = tempScore;
				if(isRoot){
					//printf("MAXIMIZER %d - TS: %d a: %d b: %d\n", depth, tempScore, alpha, beta);
					//printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

					memmove(finalMove, tempData, sizeof(Move));
				}
			}

			alpha = max(alpha, value);
			if( beta <= alpha){ 
				free(tempData); 
				break;
			}
			free(tempData);
		}
		return value;

	}else{
		value = INFINITY;
		//b = beta;											//for each child position
		while((tempData = removeFirst(moveList)) != NULL){ //for each child position

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, tempData);
			//printf("MINIMIZER Searching Move on depth %d:\n", depth);			
			value = min(value, alpha_beta(tempPosition, depth-1, alpha, beta, 1, finalMove, 0));
			beta = min(beta, value);

			if( beta <= alpha){ free(tempData); break;}
			free(tempData);
		}
		return value;
	}

	deleteList(moveList);
	free(tempPosition);
}


//Based on pseudocode and ideas from those articles
//https://en.wikipedia.org/wiki/Minimax
//https://www.researchgate.net/figure/MiniMax-Algorithm-Pseduo-Code-In-Fig-3-there-is-a-pseudo-code-for-NegaMax-algorithm_fig2_262672371
int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot){

	//In case we reach the depth we want return evaluation
	if(depth == 0){
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
			//printf("MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);
			
			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, childData);

			tempScore = max(value, minimax(tempPosition, depth - 1, FALSE, finalMove,0));
			//printf("\tDepth %d and val %d\n", depth, value);

			if(tempScore > value){
				value = tempScore;
				if(isRoot){
			//		printf("FINALL MAX \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);
					memmove(finalMove, childData, sizeof(Move));
				}
			}
			free(childData);
		}
		return value;
	}else{
		value = INFINITY;
		while((childData = removeFirst(allMoves)) != NULL){
			// printf("MIN \t\tStart from (%d, %d) and go to (%d, %d) \n", childData->tile[0][0], childData->tile[1][0], childData->tile[0][1], childData->tile[1][1]);
			//printf("depth %d and val %d\n", depth, value);
			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, childData);

			value = min(value, minimax(tempPosition, depth - 1, TRUE, finalMove,0));
			//printf("\tDepth %d and val %d\n", depth, value);

			free(childData);
		}
		return value;
	}

	deleteList(allMoves);
	free(tempPosition);
}