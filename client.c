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

#define MAX_DEPTH 8

#define INFINITY 10000000

#define MAX_TIME 0.5
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

					//myMove = initRandom(myColor);

					//maxScore = minimax(tempPosition, MAX_DEPTH, TRUE, &myMove,1);
					maxScore = alpha_beta(tempPosition, MAX_DEPTH, maxScore, -maxScore, 1, &myMove,1 );
					// maxScore = iterativeDeepeningSearch(tempPosition, &myMove);
					// maxScore = MTDFSearch(tempPosition, evaluationFunction(tempPosition) ,MAX_DEPTH, &myMove);
					// maxScore = NegaScout(tempPosition,MAX_DEPTH,-INFINITY, INFINITY, 1, &myMove);					

					printf("\t\tMAX SCORE %d\n", maxScore);

					free(tempPosition);


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
void multipleJumps(LinkedList* moveList, Move* move, short k ,short i, short j, Position *gamePos){

	move->tile[0][k] = i;
	move->tile[1][k] = j;

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
				
			}
	}

	if(moveList==NULL){ //if we can't move
		move = malloc(sizeof(Move));
		move->color = gamePos->turn;
		move->tile[0][0] = -1;
		addElement(moveList, move);
		return moveList;
	}
	return moveList;
						
}


/*
	Evaluation Function. More action in evaluation check
*/
int evaluationFunction (Position *gamePos) {
    short i,j, evaluation = 0;
    
    //Loop unrolling once again
    for (i = 0; i < BOARD_ROWS; i+=4)
    {
        for ( j = 0; j < BOARD_COLUMNS; j+=4)
        {
        	evaluation+=evaluationCheck(gamePos,i,j);
        	evaluation+=evaluationCheck(gamePos,i+1,j);
        	evaluation+=evaluationCheck(gamePos,i+2,j);
        	evaluation+=evaluationCheck(gamePos,i+3,j);

        	evaluation+=evaluationCheck(gamePos,i,j+1);
        	evaluation+=evaluationCheck(gamePos,i+1,j+1);
        	evaluation+=evaluationCheck(gamePos,i+2,j+1);
        	evaluation+=evaluationCheck(gamePos,i+3,j+1);

        	evaluation+=evaluationCheck(gamePos,i,j+2);
        	evaluation+=evaluationCheck(gamePos,i+1,j+2);
        	evaluation+=evaluationCheck(gamePos,i+2,j+2);
        	evaluation+=evaluationCheck(gamePos,i+3,j+2);

        	evaluation+=evaluationCheck(gamePos,i,j+3);
        	evaluation+=evaluationCheck(gamePos,i+1,j+3);
        	evaluation+=evaluationCheck(gamePos,i+2,j+3);
        	evaluation+=evaluationCheck(gamePos,i+3,j+3);
        }
    }
    evaluation = evaluation + gamePos->score[myColor]*120 - gamePos->score[getOtherSide(myColor)]*120;
    return evaluation;
}

/*
	The main actions regarding evaluations
	Weight 
		80 for each piece
		+100 when our pieces are covered
		-150 when enemy pieces are covered
*/
int evaluationCheck(Position *gamePos,short i, short j){
	int evaluation = 0,playerDirection = 0;

	playerDirection = myColor == WHITE ? 1 : -1; 

	//Our player give positive evaluations
	if (gamePos->board[i][j] == myColor){
       evaluation += 80;

       if(i != 0 && i!= BOARD_ROWS-1 && j != 0 && j!= BOARD_COLUMNS-1){
       		if(gamePos->board[i-playerDirection][j+1] == myColor){
       			evaluation+=100;
       		}
       		if(gamePos->board[i-playerDirection][j-1] == myColor){
       			evaluation+=100;
       		}
       }

       evaluation+=tableHeuristics[i][j];
   	}
   	//Opponent give negative evaluation
    else if (gamePos->board[i][j] == getOtherSide(myColor)){
      evaluation -= 80;
      if(i != 0 && i!= BOARD_ROWS-1 && j != 0 && j!= BOARD_COLUMNS-1){
       		if(gamePos->board[i-playerDirection][j+1] == getOtherSide(myColor)){
       			evaluation-=150;
       		}
       		if(gamePos->board[i-playerDirection][j-1] == getOtherSide(myColor)){
       			evaluation-=150;
       		}
       }

      evaluation-= tableHeuristics[i][j];

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
int quiescenceSearch(Position* gamePos){
		int i, j, playerDirection;
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


/*
	Iterative Deepening function
	https://homepages.cwi.nl/~paulk/theses/Carolus.pdf
	https://stackoverflow.com/questions/41756443/how-to-implement-iterative-deepening-with-alpha-beta-pruning
	https://github.com/nealyoung/CS171/blob/master/AI.java
*/
int iterativeDeepeningSearch(Position* gamePos, Move* finalMove)
{

	int d,res;
	/*
		Handle Time in C
		https://stackoverflow.com/questions/19084596/how-to-use-seconds-time-in-c-program-as-a-counter
	*/
	clock_t startTime;
	double timeElapsed;

	startTime = clock();
	
	for(d = 1; d <= MAX_DEPTH; d++){
		// res = MTDFSearch(gamePos, f, d, finalMove);
		// res = NegaScout(gamePos, d, -INFINITY, INFINITY, 1, finalMove);
		res = alpha_beta(gamePos, d, -INFINITY, INFINITY, 1, finalMove, 1);
		// maxScore = NegaScout(tempPosition,MAX_DEPTH,-INFINITY, INFINITY, 1, &myMove);

		timeElapsed = (double)(clock() - startTime) / CLOCKS_PER_SEC;
        if (timeElapsed >= MAX_TIME){
			printf("Max Score: %d\n", res);
			printf("Time Elapsed: %f\n", timeElapsed);
			printf("---------------\n");
			break;
        }
	}
	return res;
}



/*
	Disappointing results
	https://www.cs.unm.edu/~aaron/downloads/qian_search.pdf
	https://en.wikipedia.org/wiki/MTD-f
	https://www.chessprogramming.org/MTD(f)
	https://people.csail.mit.edu/plaat/mtdf.html ALSO TIME ITERATIVE DEEEPENING
*/


int MTDFSearch(Position* gamePos, int f, int d, Move* finalMove){
	int beta;
	int score = f;
	int upperBound = INFINITY;
	int lowerBound = - INFINITY;

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
			lowerBound = score;
		}
	}while(lowerBound < upperBound);

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


	if (moveList == NULL || moveList->head == NULL){     //Case no more moves available
		deleteList(moveList);
		return evaluationFunction(gamePos);
	}

	Position* tempPosition = malloc(sizeof(Position));
	short cur;

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

		if(alpha >= beta){
			free(tempData);
			return alpha;
		} 
		n = alpha + 1;
		free(tempData);
		
	}
	//free(tempData);
	deleteList(moveList);
	free(tempPosition);
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

	LinkedList *moveList = moveFinder(gamePos);  //available moves
	Move *tempData = NULL;


	if (moveList == NULL || moveList->head == NULL){     //Case no more moves available
		deleteList(moveList);
		return evaluationFunction(gamePos);
	}

	Position* tempPosition = malloc(sizeof(Position));
	int tempScore, value;

	if (maximizingPlayer){

		value = -INFINITY;
		while((tempData = removeFirst(moveList)) != NULL){ //for all ancestors

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);
			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, tempData);

			tempScore = alpha_beta(tempPosition, depth-1, alpha, beta, 0, finalMove,0);

			if(value < tempScore){
			 	value = tempScore;
				if(isRoot){
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
		while((tempData = removeFirst(moveList)) != NULL){ //for all ancestors

			//printf("\t\tStart from (%d, %d) and go to (%d, %d) \n", tempData->tile[0][0], tempData->tile[1][0], tempData->tile[0][1], tempData->tile[1][1]);

			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, tempData);
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

/*
	Minimax implementation
	Based on pseudocode and ideas from those articles
	https://en.wikipedia.org/wiki/Minimax
	https://www.researchgate.net/figure/MiniMax-Algorithm-Pseduo-Code-In-Fig-3-there-is-a-pseudo-code-for-NegaMax-algorithm_fig2_262672371
*/

int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot){

	//In case we reach the depth we want return evaluation
	if(depth == 0){
		return evaluationFunction(gamePos);
	}

	LinkedList *allMoves = moveFinder(gamePos);
	Move* childData = NULL;

	if (allMoves == NULL || allMoves->head == NULL){     //Case no more moves are available
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
			memmove(tempPosition, gamePos, sizeof(Position));
			doMove(tempPosition, childData);

			value = min(value, minimax(tempPosition, depth - 1, TRUE, finalMove,0));
			free(childData);
		}
		return value;
	}

	deleteList(allMoves);
	free(tempPosition);
}
