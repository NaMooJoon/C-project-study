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
	quit,
	N_op 
} commands ;

typedef enum {
	vertical,
	horizontal 
} direction ;

typedef struct car{
	int id ;
	int y1, y2 ;	// y1: the minimum of y, y2: the maximum of y
	int x1, x2 ;	// x1: the minimum of x, x2: the maximum of x
	int span ;		// the number of cells 
	direction dir ;	// the direction of the car
} car_t ;

typedef struct st {
	struct car* cars;
	struct st* parent;
} State ;


int n_cars = 0 ;
int cells[6][6] ; // cells[Y][X]
// A6 -> cells[5][0]
// F4 -> cells[3][5]
// F1 -> cells[0][5]

/* --- NOFIX */
// Queue 함수들
typedef struct node{
	State* data;
    struct node* next;
    struct node* prev;
} Node ;

typedef struct {
    Node* head;
    Node* tail;
    int count;
} Queue ;

Queue* create_queue() 
{
    Queue* new_queue = (Queue*)malloc(sizeof(Queue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->count = 0;

    return new_queue;
}
void push_queue(Queue* Q, State* state) 
{
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = state;
    if (Q->count == 0) {
        new_node->prev = NULL;
        new_node->next = NULL;
        Q->head = Q->tail = new_node;
        Q->count = 1;
        return ;
    }
    new_node->next = Q->head;
    Q->head->prev = new_node;

    Q->head = new_node;
    Q->count++;
}
    
State* pop_queue(Queue* Q) 
{
    if (Q->count == 0){
        return NULL;
    }
    Node* pop_node = Q->tail;
    State* return_data = pop_node->data;

    if (Q->count == 1) {
        Q->head = NULL;
        Q->tail = NULL;
    } else {
        Q->tail = Q->tail->prev;
        Q->tail->next = NULL;
    }
    Q->count--;
    free(pop_node);

    return return_data;
}

State* pop_queue_from_back(Queue* Q)
{
	if (Q->count == 0){
        return NULL;
    }
    Node* pop_node = Q->head;
    State* return_data = pop_node->data;

    if (Q->count == 1) {
        Q->head = NULL;
        Q->tail = NULL;
    } else {
        Q->head = Q->head->next;
        Q->head->prev = NULL;
    }
    Q->count--;
    free(pop_node);

    return return_data;
}

int is_not_empty(Queue* Q) 
{
    return Q->count;
}


State* get_new_copied_state(State* from) {
	State* return_data = malloc(sizeof(State));
	return_data->cars = malloc(sizeof(car_t) * n_cars);
	for (int i = 0; i < n_cars; i++) {
		return_data->cars[i].id = from->cars[i].id;
		return_data->cars[i].x1 = from->cars[i].x1;
		return_data->cars[i].y1 = from->cars[i].y1;
		return_data->cars[i].x2 = from->cars[i].x2;
		return_data->cars[i].y2 = from->cars[i].y2;
		return_data->cars[i].span = from->cars[i].span;
		return_data->cars[i].dir = from->cars[i].dir;
	}
	return_data->parent = NULL;

	return return_data;
}

int is_bingo(State* state) 
{
	if (state->cars[0].x2 == 5 && state->cars[0].y2 == 2) {
		return 1;
	}
	return 0;
}

int
load_game (char * filename, State** given)
{
	//FIXME
	// load_game returns 0 for a success, or return 1 for a failure.
	// Use fopen, getline, strtok, atoi, strcmp
	// Note that the last character of a line obtained by getline may be '\n'.
	FILE * fp = fopen(filename, "r");
	char *line = NULL;
    size_t len = 0;
    // ssize_t read;
	char * data;
	int spanOfCar = 0;
	int i;

	// file read fail
	if (fp == NULL) {
		fclose(fp);
		return 1;
	}

	// read the number of cars
	if (getline(&line, &len, fp) == -1)  	
		return 1;
	n_cars = atoi(line);
	if (n_cars < 1 || n_cars > 36)				
		return 1;


	// alloc the cars
	car_t* cars = malloc(sizeof(car_t) * n_cars);

	for (i = 0; i < n_cars; i++) {
		if (getline(&line, &len, fp) == -1)
			return 1;

		cars[i].id = i + 1;
		// cell:
		data = strtok(line, ":");
		cars[i].x1 = data[0] - 'A';
		cars[i].y1 = 6 - (data[1] - '0');
		if (strlen(data) != 2)				
			return 1;
		if (cars[i].x1 < 0  || cars[i].x1 > 5) 
			return 1;
		if (cars[i].y1 < 0  || cars[i].y1 > 5) 
			return 1;
		
		// :direction:
		data = strtok(NULL, ":"); 
		cars[i].dir = (strcmp(data, "vertical") == 0)	 ? vertical
					: (strcmp(data, "horizontal") == 0) ? horizontal : -1; 
		if (cars[i].dir == -1) 
			return 1;

		// :span
		data = strtok(NULL, ":");
		cars[i].span = atoi(data);
		if (cars[i].span < 1 || cars[i].span > 6) 
			return 1;
		cars[i].x2 = (cars[i].dir == horizontal) ? cars[i].x1 + cars[i].span - 1 : cars[i].x1;
		cars[i].y2 = (cars[i].dir == vertical)	 ? cars[i].y1 + cars[i].span - 1 : cars[i].y1;
    }
	(*given)->cars = cars;
	(*given)->parent = NULL;

	free(line);
	fclose(fp);
	
	return 0;
}

void
display ()
{
	printf("\n");
	int i, j;
	//FIXME
	for (i = 0; i < 6; i++) {
		for (j = 0 ; j < 6; j++) {
			if (cells[i][j] == 0) 
				printf("+ ");
			else 
				printf("%d ", cells[i][j]);
		}
		printf("\n");
	}/* The beginning state of board1.txt must be shown as follows: 
 	 + + 2 + + +
 	 + + 2 + + +
	 1 1 2 + + +
	 3 3 3 + + 4
	 + + + + + 4
	 + + + + + 4
	*/	
}

int 
update_cells (State* state)
{
	memset(cells, 0, sizeof(int) * 36) ; // clear cells before the write.

	//FIXME
	// return 0 for sucess
	// return 1 if the given car information (cars) has a problem
	for (int i = 0; i < n_cars; i++) {
		int x = state->cars[i].x1;
		int y = state->cars[i].y1;
		for (int j = 0; j < state->cars[i].span; j++) {
			if (x > 6 || y > 6) 	
				return 1;
			if (cells[y][x] != 0)	
				return 1;
			cells[y][x] = state->cars[i].id;
			if (state->cars[i].dir == horizontal)
				x++;
			else
				y++;
		}
	}
	return 0;

}

int
move (State* state, int id, int op) 
{
	car_t* cars = state->cars;
	switch (op) {
			case left:
				if (cars[id].dir != horizontal) 			 return 0;
				if (cars[id].x1 == 0)						 return 0;
				if (cells[cars[id].y1][cars[id].x1-1] != 0)  return 0;
				cars[id].x1--;
				cars[id].x2--;
				break;

			case right:
				if (cars[id].dir != horizontal) 			 return 0;
				if (cars[id].x2 == 5)						 return 0;
				if (cells[cars[id].y2][cars[id].x2+1] != 0)  return 0;
				cars[id].x1++;
				cars[id].x2++;
				break;

			case up:
				if (cars[id].dir != vertical) 			 	 return 0;
				if (cars[id].y1 == 0)						 return 0;
				if (cells[cars[id].y1-1][cars[id].x1] != 0)  return 0;
				cars[id].y1--;
				cars[id].y2--;
				break;

			case down:
				if (cars[id].dir != vertical) 			 	 return 0;
				if (cars[id].y2 == 5)						 return 0;
				if (cells[cars[id].y2+1][cars[id].x2] != 0)  return 0;
				cars[id].y1 ++;
				cars[id].y2 ++;
				break;

			default :
				return 0;
	}
	return 1;
}


int is_same_state (State* state1, State* state2)
{
	for (int i = 0; i < n_cars; i++) {
		if (state1->cars[i].id != state2->cars[i].id)
			return 0;
		if (state1->cars[i].x1 != state2->cars[i].x1 || state1->cars[i].y1 != state2->cars[i].y1)
			return 0;
		if (state1->cars[i].x2 != state2->cars[i].x2 || state1->cars[i].y2 != state2->cars[i].y2)
			return 0;
	}
	return 1;
}

int not_visited (Queue* V, State* new_state)
{	
	if (is_not_empty(V)) {
		for (Node* curr = V->head; curr != NULL; curr = curr->next) {
			if (is_same_state(curr->data, new_state))
				return 0;
		}
	}
	return 1;
}

void next_state (Queue* Q, Queue* V, State* curr_state)
{
	update_cells(curr_state);
	// display();
	for (int i = 0; i < n_cars; i++) {
		if (curr_state->cars[i].dir == horizontal) {
			// left
			State* left_case = get_new_copied_state(curr_state);
			if (move(left_case, i, left)) {
				if (not_visited(V, left_case)) {
					left_case->parent = curr_state;
					push_queue(V, left_case);
					push_queue(Q, left_case);
				} else {
					free(left_case);
				}
			}
			// right
			State* right_case = get_new_copied_state(curr_state);
			if (move(right_case, i, right)) {
				if (not_visited(V, right_case)) {
					right_case->parent = curr_state;
					push_queue(V, right_case);
					push_queue(Q, right_case);
				} else {
					free(right_case);
				}
			}
		} else if (curr_state->cars[i].dir == vertical) {
			// up
			State* up_case = get_new_copied_state(curr_state);
			if (move(up_case, i, up)) {
				if (not_visited(V, up_case)) {
					up_case->parent = curr_state;
					push_queue(V, up_case);
					push_queue(Q, up_case);
				} else {
					free(up_case);
				}
			}
			// down
			State* down_case = get_new_copied_state(curr_state);
			if (move(down_case, i, down)) {
				if (not_visited(V, down_case)) {
					down_case->parent = curr_state;
					push_queue(V, down_case);
					push_queue(Q, down_case);
				} else {
					free(down_case);
				}
			}
		}
	}
}

void start_computer_game()
{
	char filename[128] ;
	Queue* Q = create_queue();
	Queue* V = create_queue();
	State* given ;

	printf("Input the game board..\n");
	scanf("%s", filename) ;
	if (load_game(filename, &given)) {
		printf ("\ninvalid data..\n");
	}
	
	push_queue(Q, given);
	push_queue(V, given);
	
	while (is_not_empty(Q))
	{
		State* curr_state = pop_queue(Q);
		if (is_bingo(curr_state)) {
			printf("Bingo!!\n");
			while (is_not_empty(V))
				pop_queue(V);
			for (State* data = curr_state; data != NULL; data = data->parent){
				printf("data address:%p\n", data);
				push_queue(V, data);
			}
			break;
		}
		next_state(Q, V, curr_state);
	}

	while (is_not_empty(V)) {
		update_cells(pop_queue_from_back(V));
		display();
	}

	if (!is_not_empty(Q))
		printf("?\n");


	free(Q);
	free(V);
}


int
main ()
{
	start_computer_game();

	return 0;
}
