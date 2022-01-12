#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* NOFIX --- */

typedef enum {
	start,
	left,
	right,
	up,
	down,
	auto,
	quit,
	N_op 
} commands ;

typedef enum {
	vertical,
	horizontal 
} direction ;

typedef struct {
	int id ;
	int y1, y2 ;	// y1: the minimum of y, y2: the maximum of y
	int x1, x2 ;	// x1: the minimum of x, x2: the maximum of x
	int span ;		// the number of cells 
	direction dir ;	// the direction of the car
} car_t ;

char * op_str[N_op] = {
	"start",
	"left",
	"right",
	"up",
	"down",
	"quit"
} ;

int n_cars = 0 ;
car_t * cars = 0x0 ;
int cells[6][6] ; // cells[Y][X]
// A6 -> cells[5][0]
// F4 -> cells[3][5]
// F1 -> cells[0][5]
int memo_cells[200][6][6]

/* --- NOFIX */


commands
get_op_code (char * s)
{
	// return the corresponding number for the command given as s.
	// FIXME
	if ( strcmp( s, "start" )	== 0 )	return start;
	if ( strcmp( s, "left" ) 	== 0 ) 	return left;
	if ( strcmp( s, "right" ) 	== 0 ) 	return right;
	if ( strcmp( s, "up" ) 		== 0 ) 	return up;
	if ( strcmp( s, "down" ) 	== 0 ) 	return down;
	if ( strcmp( s, "auto") 	== 0 )	return auto;
	if ( strcmp( s, "quit" ) 	== 0 ) 	return quit;

	return N_op;
}


int
load_game (char * filename)
{
	//FIXME
	// load_game returns 0 for a success, or return 1 for a failure.
	// Use fopen, getline, strtok, atoi, strcmp
	// Note that the last character of a line obtained by getline may be '\n'.
	FILE * fp = fopen ( filename, "r" );
	char *line = NULL;
    size_t len = 0;
    ssize_t read;
	char * data;
	int spanOfCar = 0;

	// file read fail
	if ( fp == NULL )
		return 1;

	// read the number of cars
	if (( read = getline(&line, &len, fp)) == -1 )  return 1;
	n_cars = atoi ( line );
	if ( n_cars < 1 || n_cars > 36 )				return 1;


	// alloc the cars
	cars = malloc ( sizeof(car_t) * n_cars );

	for ( int i = 0; i < n_cars; i++ ) {
		// read one line.
		if ((read = getline(&line, &len, fp)) == -1)
			return 1;

		// set car id.
		cars[i].id = i + 1;

		// cell:
		data = strtok ( line, ":" );
		cars[i].x1 = data[0] - 'A';
		cars[i].y1 = 6 - (data[1] - '0');
		if ( strlen(data) != 2 )				goto _err_load_file_;
		if ( cars[i].x1 < 0  || cars[i].x1 > 5) return 1;
		if ( cars[i].y1 < 0  || cars[i].y1 > 5) return 1;

		// :direction:
		data = strtok ( NULL, ":" ); 
		cars[i].dir = ( strcmp( data, "vertical" ) == 0 ) ? vertical
					: ( strcmp( data, "horizontal") == 0) ? horizontal : 3; 
		if ( cars[i].dir == 3 ) return 1;

		// :span
		data = strtok ( NULL, ":" );
		cars[i].span = atoi ( data );
		if ( cars[i].span < 1 || cars[i].span > 6) return 1;
		cars[i].x2 = ( cars[i].dir == horizontal ) 	? cars[i].x1 + cars[i].span - 1 : cars[i].x1;
		cars[i].y2 = ( cars[i].dir == vertical ) 	? cars[i].y1 + cars[i].span - 1 : cars[i].y1;

    }

	free ( line );
	fclose ( fp );

	return 0;

_err_load_file_:
	free ( line );
	fclose ( fp );

	return 1;
}

void
display ()
{
	/* The beginning state of board1.txt must be shown as follows: 
 	 + + 2 + + +
 	 + + 2 + + +
	 1 1 2 + + +
	 3 3 3 + + 4
	 + + + + + 4
	 + + + + + 4
	 
	*/

	//FIXME
	for ( int i = 0; i < 6; i++ ) {
		for ( int j = 0 ; j < 6; j++ ) {
			if ( cells[i][j] == 0 ) 
				printf("+ ");
			else 
				printf("%d ", cells[i][j]);
		}
		printf("\n");
	}
}

int 
update_cells ()
{
	memset(cells, 0, sizeof(int) * 36) ; // clear cells before the write.

	//FIXME
	// return 0 for sucess
	// return 1 if the given car information (cars) has a problem
	for ( int i = 0; i < n_cars; i++ ) {
		int x = cars[i].x1;
		int y = cars[i].y1;
		for ( int j = 0; j < cars[i].span; j++ ) {
			if ( x > 6 || y > 6 ) 	return 1;
			if ( cells[y][x] != 0 )	return 1;
			cells[y][x] = cars[i].id;
			( cars[i].dir == horizontal )? x++ : y++;
		}
	}
	return 0;
}

int
move (int id, int op) 
{
	//FIXME
	// move returns 1 when the given input is invalid.
	// or return 0 for a success.

	// Update cars[id].x1, cars[id].x2, cars[id].y1 and cars[id].y2
	//   according to the given command (op) if it is possible.

	// The condition that car_id can move left is when 
	//  (1) car_id is horizontally placed, and
	//  (2) the minimum x value of car_id is greather than 0, and
	//  (3) no car is placed at cells[cars[id].y1][cars[id].x1-1].
	// You can find the condition for moving right, up, down as
	//   a similar fashion.
	id = id - 1;
	switch ( op ) {
			case left:
				if ( cars[id].dir != horizontal ) 			 return 1;
				if ( cars[id].x1 == 0 )						 return 1;
				if ( cells[cars[id].y1][cars[id].x1-1] != 0) return 1;
				cars[id].x1 --;
				cars[id].x2 --;
				break;

			case right:
				if ( cars[id].dir != horizontal ) 			 return 1;
				if ( cars[id].x2 == 5 )						 return 1;
				if ( cells[cars[id].y2][cars[id].x2+1] != 0) return 1;
				cars[id].x1 ++;
				cars[id].x2 ++;
				break;

			case up:
				if ( cars[id].dir != vertical ) 			 return 1;
				if ( cars[id].y1 == 0 )						 return 1;
				if ( cells[cars[id].y1-1][cars[id].x1] != 0) return 1;
				cars[id].y1 --;
				cars[id].y2 --;
				break;

			case down:
				if ( cars[id].dir != vertical ) 			 return 1;
				if ( cars[id].y2 == 5 )						 return 1;
				if ( cells[cars[id].y2+1][cars[id].x2] != 0) return 1;

				cars[id].y1 ++;
				cars[id].y2 ++;
				break;

			default :
				return 1;
		}
	return 0;
}

int autoSolve(int id) 
{
	id = id - 1;
	// 움직일 수 있는 블록들 찾기.
}

int
main ()
{
	char buf[128] ;
	int op ;
	int id ;

	while (1) {
		scanf("%s", buf) ;

		switch (op = get_op_code(buf)) {
			case start:
				scanf("%s", buf) ;
				if ( load_game(buf) ) { printf ("\ninvalid data..\n"); break; }
				if ( update_cells() ) { printf ("\ninvalid data..\n"); break; }
				display() ;
				break;

			case left:
			case right:
			case up:
			case down:
				scanf("%d", &id) ;
				if ( move(id, op) ) { printf ("\nimpossible..\n"); break; }
				update_cells() ;
				display() ;
				break;
			//FIXME
			case auto:
				autoSolve();
				break;
			case N_op:
				printf ( "\ninvalid command..\n");
				break;
		}
		if ( cars == 0x0 ) continue;
		if ( (cars[0].x1 == 5 && cars[0].y1 == 2)  
				||	(cars[0].x2 == 5 && cars[0].y2 == 2) ) {
			printf ( "\ndone!!!\n");
			printf ( "Input the new board..\n");
			continue;
		}
	}

	free(cars);
}
