#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define TRUE        1
#define FALSE       0

typedef enum {
    person,
    wolf,
    sheep,
    cabbage,
    N_sub
} Subject ;

char * sub_name[N_sub] = {
	"*person",
	"wolf",
	"sheep",
	"cabbage",
} ;

typedef enum {
    left,
    right,
    N_Loc
} Loc ;

typedef struct state{
    Loc person;
    Loc wolf;
    Loc sheep;
    Loc cabbage;
    struct state* parent;
} State ;

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

// Queue 함수들
Queue* create_queue() ;
void push_queue(Queue* Q, State* state) ;
State* pop_queue(Queue* Q) ;
int is_not_empty(Queue* Q) ;

State* make_new_state (Loc person, Loc wolf, Loc sheep, Loc cabbage) ;

// check whether same location { return TRUE=1, FALSE=0 }
int same_location (Subject s1, Subject s2) ;
int not_same_location (Subject s1, Subject s2) ;
// check whether same state { return TRUE=1, FALSE=0 }
int same_state (State* s1, State* s2) ;
int not_same_state (State* s1, State* s2) ;
// for printing result
void print_state(State* state) ;
void print_route(State* curr) ;
int is_even (int num) ;


int main (void) 
{
    State* start = make_new_state(left, left, left, left);
    State* answer  = make_new_state(right, right, right, right);
    State* result_state;

    Queue* Q = create_queue();
    State** visit_state = malloc(sizeof(State*) * (N_sub * N_sub + 1));
    int v_count = 0;

    push_queue(Q, start);

    while (is_not_empty(Q)) 
    {
        State* curr_state = pop_queue(Q);
        visit_state[v_count++] = curr_state;
        if (same_state(curr_state, answer)) {
            result_state = curr_state;
            break;
        }
        Loc next_location = (curr_state->person == left)? right : left;

        for (int i = 0; i < N_sub; i++) {
            State* next_state = make_new_state(next_location, curr_state->wolf, curr_state->sheep, curr_state->cabbage);
            int check = 0;
            switch (i) {
                case person:
                    check++;
                    break;
                case wolf:
                    if (curr_state->person == curr_state->wolf) {
                        next_state->wolf = next_location;
                        check++;
                    }
                    break;
                case sheep:
                    if (curr_state->person == curr_state->sheep) {
                        next_state->sheep = next_location;
                        check++;
                    }
                    break;
                case cabbage:
                    if (curr_state->person == curr_state->cabbage) {
                        next_state->cabbage = next_location;
                        check++;
                    }
                    break;
            }
            if (not_same_location(next_state->person, next_state->wolf)
                && same_location(next_state->wolf, next_state->sheep)) {
                check = 0;
            }
            if (not_same_location(next_state->person, next_state->sheep)
                && same_location(next_state->sheep, next_state->cabbage)) {
                check = 0;
            }
            if (check) {
                int not_visited = 1;
                for (int j = 0; j < v_count; j++) {
                    if (same_state(visit_state[j], next_state)) {
                        not_visited = 0;
                        break;
                    }
                }
                if (not_visited) {
                    next_state->parent = curr_state;
                    push_queue(Q, next_state);
                } else {
                    free(next_state);
                }
            }
        }
    }

    printf("[person,wolf,sheep,cabbage]\n");
    print_route(result_state);
    printf("bingo!!\n");

    while (is_not_empty(Q)) {
        State* dump = pop_queue(Q);
        int visited = 1;
        for (int j = 0; j < v_count; j++) {
            if (same_state(visit_state[j], dump)) {
                visited = 0;
                break;
            }
        }
        if (visited) 
            free(dump);
    }
    free(Q);
    for (int i = 0; i < v_count; i++) {
        State* dump = visit_state[i];
        free(dump);
    }
    free(visit_state);

    return 0;
}





int same_location (Subject s1, Subject s2) 
{
    return (s1 == s2)? TRUE : FALSE;
}
int not_same_location (Subject s1, Subject s2) 
{
    return (s1 != s2)? TRUE : FALSE;
}

State* make_new_state (Loc person, Loc wolf, Loc sheep, Loc cabbage)
{
    // 이 함수에서 State를 반환.
    State* newState = malloc(sizeof(State));
    newState->person    = person;
    newState->wolf      = wolf;
    newState->sheep     = sheep;
    newState->cabbage   = cabbage;
    newState->parent    = NULL;

    return newState;
}

int not_same_state (State* s1, State* s2) 
{
    if (same_location(s1->person, s2->person) && same_location(s1->wolf,s2->wolf)
        && same_location(s1->sheep, s2->sheep) && same_location(s1->cabbage,s2->cabbage)) {
            return FALSE;
    }
    return TRUE;
}
int same_state (State* s1, State* s2) 
{
    if (same_location(s1->person, s2->person) && same_location(s1->wolf,s2->wolf)
        && same_location(s1->sheep, s2->sheep) && same_location(s1->cabbage,s2->cabbage)) {
            return TRUE;
    }
    return FALSE;
}

int is_even (int num) 
{
    return (num%2 == 0)? TRUE : FALSE ;
}

void print_state(State* state) 
{
    int on_left[N_sub] = { FALSE, };

    if (state->person == left)
        on_left[person] = TRUE;
    if (state->wolf == left)
        on_left[wolf] = TRUE;
    if (state->sheep == left)
        on_left[sheep] = TRUE;
    if (state->cabbage == left)
        on_left[cabbage] = TRUE;
    
    printf("===================================\n");
    for (int i = 0; i < N_sub; i++) {
        if (on_left[i] == TRUE) {
            printf("    %-8s  ", sub_name[i]);
            if (is_even(i))
                printf("( ( (");
            else
                printf(" ) ) )");
        } else {
            if (is_even(i))
                printf("              ( ( ( ");
            else
                printf("               ) ) )");
            printf("%10s", sub_name[i]);
        }
        printf("\n");
    }
    printf("===================================\n");
}

void print_route(State* curr) 
{
    if (curr->parent != NULL) {
        print_route(curr->parent);
        // usleep(500000);
        if (curr->person == right) {
            printf("          Let's go to right\n");
        } else {
            printf("          Let's go to left\n");
        }
        // usleep(1000000);
    }
    print_state(curr);
}




// Queue 함수들
Queue* create_queue() 
{
    Queue* newQueue = (Queue*)malloc(sizeof(Queue));
    newQueue->head = NULL;
    newQueue->tail = NULL;
    newQueue->count = 0;

    return newQueue;
}
void push_queue(Queue* Q, State* state) 
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = state;
    if (Q->count == 0) {
        newNode->prev = NULL;
        newNode->next = NULL;
        Q->head = Q->tail = newNode;
        Q->count = 1;
        return ;
    }
    newNode->next = Q->head;
    Q->head->prev = newNode;

    Q->head = newNode;
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

int is_not_empty(Queue* Q) 
{
    return Q->count;
}

