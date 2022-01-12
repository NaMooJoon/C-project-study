#define _GNU_SOURCE 

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

typedef struct {
   int id ;
   int y1, y2 ;   // y1: the minimum of y, y2: the maximum of y
   int x1, x2 ;   // x1: the minimum of x, x2: the maximum of x
   int span ;      // the number of cells 
   direction dir ;   // the direction of the car
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

/* --- NOFIX */


commands
get_op_code (char * s)
{
    printf("get_op_code started!!\n");
    commands command;

    if (strcmp(s, "start")==0) 
        command = start;
    if (strcmp(s, "left")==0) 
        command = left;
    if (strcmp(s, "right")==0) 
        command = right;
    if (strcmp(s, "up")==0) 
        command = up;
    if (strcmp(s, "down")==0) 
        command = down;
    if (strcmp(s, "quit")==0) 
        command = quit;

    return command;

    // return the corresponding number for the command given as s.
   // FIXME
}

int
load_game (char * filename)
{   
    printf("load_game started!!\n");

    char* line;
    FILE* fp = fopen(filename, "r");
    car_t * car;
    size_t len = 0;
    //ssize_t re;   //ssize_t? int?..
    int id=0;
    char* cut;

    line = malloc(1024);
    len = 1024;

    fscanf(fp, "%d", &n_cars);
    cars = (car_t *) malloc(sizeof(car_t) * (n_cars + 1));

    getline(&line, &len, fp);

    while((getline(&line, &len, fp))!=-1){
        //location
        id++;
        cut = strtok(line, ":\n");
        cars[id].x1 = cut[0]-'A';
        if (atoi(cut+1)>6 || atoi(cut+1)<1) {
            printf("%d", atoi(cut+1));
            return 1;
        }
        cars[id].y2= (cut[1]-'1');
        printf("x1: %d\n", cars[id].x1);
        //dir
        cut = strtok(0x0, ":\n");
        if (strcmp(cut, "horizontal")==0) 
            cars[id].dir = horizontal;
        else if (strcmp(cut, "vertical")==0) 
            cars[id].dir = vertical;
        else 
            return 1;                          
        printf("dir completed\n");


        //span
        cut = strtok(0x0, ":\n");


        cars[id].span = atoi(cut);
        printf("%d\n", cars[id].span);
        if (cars[id].dir==horizontal){
            if (cars[id].x1+cars[id].span-1 > 5) {
                printf("cars[id].x1+cars[id].span: %d\n", cars[id].x1+cars[id].span);
                return 1; 
            }
            cars[id].x2=cars[id].x1+cars[id].span - 1;
            cars[id].y1=cars[id].y2;
        }
        else if (cars[id].dir==vertical){
            if (cars[id].y2-(cars[id].span-1) < 0) {
                printf("cars[id].y2-cars[id].span: %d\n", cars[id].y2+cars[id].span-1);
                return 1;
            }
            cars[id].y1=cars[id].y2 - cars[id].span + 1;
            cars[id].x2=cars[id].x1;
        }

        printf("end of line: %d %d\n", cars[id].x1,cars[id].y1);
        printf("end of line: %d %d\n", cars[id].x2,cars[id].y2);

        
    
    }
    free (line);
    line = NULL;
    return 0;

    //goto 써봅시다

   //FIXME
   // load_game returns 0 for a success, or return 1 for a failure.
   // Use fopen, getline, strtok, atoi, strcmp
   // Note that the last character of a line obtained by getline may be '\n'.
}

void
display ()
{
    for (int i = 0; i<=5; i++){
        for (int j = 0; j<=5; j++){
            if(cells[i][j]==0)
                printf("+ ");
            else 
                printf("%d ", cells[i][j]);
        }
        printf("\n");
    }


   /* The beginning state of board1.txt must be shown as follows: 
     + + 2 + + +
     + + 2 + + +
    1 1 2 + + +
    3 3 3 + + 4
    + + + + + 4
    + + + + + 4
   */

   //FIXME
}

int 
update_cells ()
{
    printf("update_cells started!!\n");

   memset(cells, 0, sizeof(int) * 36) ; // clear cells before the write.

    for(int i=1; i<=n_cars; i++){
        if(cars[i].dir==vertical){
            for(int j=cars[i].y1; j<=cars[i].y2; j++){
                cells[5-j][cars[i].x1]=i;
            }
            //return 0;
        }
        else if(cars[i].dir==horizontal){
            for(int j=cars[i].x1; j<=cars[i].x2; j++){
                cells[5-cars[i].y1][j]=i;
            }
            //return 0;
        }
    }
    
    return 0;
   //FIXME
   // return 0 for success
   // return 1 if the given car information (cars) has a problem
}

int
move (int id, int op) 
{
    printf("move started!!\n");


    switch (op){
        case 1:
        if(cars[id].dir==vertical) 
            return 1;
        if(cars[id].x1==0) 
            return 1;
        if(cells[cars[id].y1][cars[id].x1-1]!=0) 
            return 1;
        else{
            cars[id].x1--;   
            return 0;
        }

        case 2:
        if(cars[id].dir==vertical) 
            return 1;
        if(cars[id].x2==5) 
            return 1;
        if(cells[cars[id].y1][cars[id].x2+1]!=0) 
            return 1;
        else{
            cars[id].x2++;
            return 0;
        }

        case 3:
        if(cars[id].dir==horizontal) 
            return 1;
        if(cars[id].y2==5) 
            return 1;
        if(cells[cars[id].y2+1][cars[id].x1]!=0) 
            return 1;
        else{
            cars[id].y2++;
            return 0;
        }

        case 4:
        if(cars[id].dir==horizontal) 
            return 1;
        if(cars[id].y1==0) 
            return 1;
        if(cells[cars[id].y1-1][cars[id].x1]!=0) 
            return 1;
        else{
            cars[id].y1--;
            return 0;
        }
    }
    return 1;
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
}

int
main ()
{

   char buf[128] ;
   int op ;
   int id ;

   while (1) {

        if (cars != 0X0){
            if (cars[1].x2==5){
            printf("done!");
            }
        }
        
        scanf("%s", buf) ;
        

      switch (op = get_op_code(buf)) {
         case start:
            scanf("%s", buf) ;
            if (load_game(buf)==1) {
                    printf("invalid file!");
                    continue;
                } 
            if(update_cells()==1){
                    printf("failed to update the board, please check your file");
                    continue;
                }
            display() ;
                break;

         case left:
         case right:
         case up:
         case down:
            scanf("%d", &id) ;
            if(move(id, op)==1){
                    printf("invalid input!");
                }
                
            if(update_cells()==1){
                    printf("failed to update the board, please check your file");
                    continue;
                }
            display() ;
                break;
            case quit:
                exit(0);
         //FIXME
      }
   }
}