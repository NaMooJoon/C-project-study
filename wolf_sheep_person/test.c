#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
typedef enum {
	left, 
	right 
} loc ; 

typedef struct st{ 
    loc wolf;
    loc sheep;
    loc cabbage;
    loc person;
    struct st* prev; 
} State ; 

typedef struct node{
    State data; 
    struct node* next; 
} Node ; 

typedef struct {
    Node* front;
    Node* rear;
    int cnt;
} Que ;

struct node *head=NULL;
struct node *curr;
int visit[16]={0,};

void push(Node** head, State s){
   Node *new=malloc(sizeof(struct node));
   new->data=s;
   new->next=*head;
   *head=new;
}

// int pop(){
//     struct node *dump;
//     if(head==NULL){
//         return 1;
//     }else{
//         dump=head;
//         printf("Poped: %d %d %d %d\n", dump->data.wolf,dump->data.sheep,dump->data.cabbage,dump->data.person);
//         head=head->next;
//         free(dump);
//     }
//     return 0;
// }

int node_current_status (Node* head){
	/* The beginning state of board1.txt must be shown as follows: 
 	<LLLL>
	*/
    if(head==NULL){
        return 1; 
    }else{
        printf("<%d %d %d %d>",head->data.wolf,head->data.sheep,head->data.cabbage,head->data.person);
        return 0;
    }
}

void node_display_list (){
	/*
    <LLLL> - <LRLR> - <LRLL> - <LRRR> - <LLRL> - <RLRR> - <RLRL> - <RRRR>
	*/
    struct node *point=head;
    while(point!=NULL){
        node_current_status(point);
        printf("  ");
        point=point->next;
    }
    printf("\n");
}

// queue <- 열혈 자료구조 C 참조(p.269)
int q_isempty(Que* q){
    if(q->front==NULL){
        return 1;
    }else{
        return 0;
    }
}
void newque (Que* q){
    q->front=NULL;
    q->rear=NULL;
}
void enque (Que* q, State s){ 
    Node *node = (Node*)malloc(sizeof(Node));
    node->data = s;
    node->next = NULL;
    if(q_isempty(q)){
        // printf("en:empty\n");
        q->cnt=0;
        q->front=node;
        q->rear=node;
    }else{
        // printf("en:new <%d %d %d %d>\n",s.wolf,s.sheep,s.cabbage,s.person);
        q->rear->next=node;
        q->rear=q->rear->next;
    }
    q->cnt++;
}
State deque (Que* q){
    Node *dump;
    State s;
    if(q_isempty(q)){
        // printf("de:empty\n");
    }else{
        dump=q->front;
        s=dump->data;
        q->front=q->front->next;
        printf("de: <%d %d %d %d>\n",dump->data.wolf,dump->data.sheep,dump->data.cabbage,dump->data.person);
        free(dump);
        q->cnt--;
    }
    return s;
}
void display (Que* q)
{
    Node* cur;
    cur=q->front;
    while (cur!=NULL){
        printf("show: <%d %d %d %d>\n",cur->data.wolf,cur->data.sheep,cur->data.cabbage,cur->data.person);
        cur=cur->next;
    }
}
int validate(State s){
    if((s.wolf!=s.sheep)&&(s.sheep!=s.cabbage)||(s.wolf==s.person)&&(s.wolf==s.sheep)||(s.cabbage==s.person)&&(s.cabbage==s.sheep)){
        // printf("validate success\n");
        return 0;
    }else{
        // printf("validate fail\n");
        return 1;
    }
}

void createnext (State s){ // ->큐로 바로 넘기기,스테이트 파라미터, 큐로 바로 넘기기
    //0: person only, 1: person+wolf, 2:person+sheep, 3:person+cabbage
    State new_a={s.wolf,s.sheep,s.cabbage,!(s.person)};
    if(validate(new_a)==0){
        push(&head,new_a);
        // printf("check1 <%d%d%d%d>\n",s.wolf,s.sheep,s.cabbage,!(s.person));
    }
    State new_b={!(s.wolf),s.sheep,s.cabbage,!(s.person)};
    if(validate(new_b)==0){
        push(&head,new_b);
        // printf("check2 <%d%d%d%d>\n",!(s.wolf),s.sheep,s.cabbage,!(s.person));
    }
    State new_c={s.wolf,!(s.sheep),s.cabbage,!(s.person)};
    if(validate(new_c)==0){
        push(&head,new_c);
        // printf("check3 <%d%d%d%d>\n",s.wolf,!(s.sheep),s.cabbage,!(s.person));
    }
    State new_d={s.wolf,s.sheep,!(s.cabbage),!(s.person)};
    if(validate(new_d)==0){
        push(&head,new_d);
        // printf("check4 <%d%d%d%d>\n",s.wolf,s.sheep,!(s.cabbage),!(s.person));
    }
}

int hash(State s){
    int key = s.wolf*8+s.sheep*4+s.cabbage*2+s.person*1;
    return key;
}

int bfs (Que* q){
    State rst={right,right,right,right};
    if(head==NULL){
        return 1;
    }
    struct node* start;
    while (!q_isempty(q)) {
        // printf("============\n");
        start = head;
        State s;
        s=deque(q);
        State* ss = malloc(sizeof(State));
        ss->wolf = s.wolf;
        ss->sheep = s.sheep;
        ss->cabbage = s.cabbage;
        ss->person = s.person;
        ss->prev = s.prev;

        //printf("state address:%p\n", ss);
        // printf("empty: %d\n",q_isempty(q));
        if((s.wolf==rst.wolf)&&(s.sheep==rst.sheep)&&(s.cabbage==rst.cabbage)&&(s.person==rst.person)){
            printf("Bingo================\n");
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            ss = ss->prev;
            printf("visit::<%d%d%d%d>\n",ss->wolf,ss->sheep,ss->cabbage,ss->person);
            printf("BINGO");
            printf("BINGO");
            return 0;
        }

        createnext(s);

        for(start = head; start != NULL; start = start->next){
            if(visit[hash(start->data)]!=1){
                start->data.prev = ss;
                enque(q,start->data);
                visit[hash(start->data)]=1;
                // start->next->data.prev=start; 
                // printf("visit::<%d%d%d%d>\n",start->data.wolf,start->data.sheep,start->data.cabbage,start->data.person);
            }
        }
    }
    // for(curr=start;curr!=head; curr=curr->data.prev){
    //     printf("<%d%d%d%d>\n",curr->data.wolf,curr->data.sheep,curr->data.cabbage,curr->data.person);
    // }
    return 1; 
}

int main (){
	char buf[128] ;
    Que q;
    State s={left,left,left,left};
    newque(&q);
    push(&head,s);
    enque(&q,s);
    visit[hash(s)]=1;
    printf("<%d%d%d%d>\n",s.wolf,s.sheep,s.cabbage,s.person);

    bfs(&q);

    return 0;
}