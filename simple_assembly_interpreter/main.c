#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE      100

typedef enum {
    READ,
    PRINT,
    ASSIGN,
    DEFINE,
    ADD,
    MINUS,
    MOD,
    EQ,
    LESS,
    JUMP,
    TERM,
    N_op
} InstructionType ;

char* op_str[N_op] = {
    "READ",
    "PRINT",
    "ASSIGN",
    "DEFINE",
    "ADD",
    "MINUS",
    "MOD",
    "EQ",
    "LESS",
    "JUMP",
    "TERM"
};

typedef struct {
    InstructionType op ;
    int md ; 
    int mx ;
    int my ;
} Instruction ;

typedef union {
    Instruction instruction ;
    int integer ;
} Value ;

typedef enum {
    EMPTY,
    INUSE
} MemStatus ;

typedef struct {
    Value value ;
    MemStatus status ;
} Data ;
// Mem : Addr -> value

Data memory[MEM_SIZE] = {{{0, 0, 0, 0}, EMPTY},} ;

InstructionType get_op_code (char* s)
{
    for (int i = 0 ; i < N_op ; i++) {
        if (strcmp(op_str[i], s) == 0)
            return i ;
    }
    return N_op;
}

int read_code (char* filename) 
{
    FILE* fp = fopen(filename, "r") ;
    char *line = NULL ;
    size_t len = 0 ;
    char* read_point ;
    int address ;
    InstructionType instruction ;
    InstructionType op ;
    int md, mx, my ;

    if (!fp) { 
        fprintf(stderr, "FILE READ ERROR!!\n") ;
        goto _error_file_open_ ;
    }

    while (getline(&line, &len, fp) != -1) 
    {
        line[strlen(line)-1] = '\0' ;    // remove '\n'

        read_point = strtok(line, " ") ;
        address = atoi(read_point) ;
        read_point = strtok(NULL, " ") ;
        if ('0' <= read_point[0] && read_point[0] <= '9') {
            memory[address].value.integer = atoi(read_point) ;
            continue ;
        }
        instruction = get_op_code(read_point) ; 
        switch (instruction)
        {
            case READ:
            case PRINT:
                read_point = strtok(NULL, " ") ;
                op = instruction ;
                md = atoi(read_point) ;
                break;
            case ASSIGN:
            case DEFINE:
            case JUMP:
                read_point = strtok(NULL, " ");
                md = atoi(read_point);
                read_point = strtok(NULL, " ");
                mx = atoi(read_point);
                my = 0;
                break;
            case ADD:
            case MINUS:
            case MOD:
            case EQ:
            case LESS:
                read_point = strtok(NULL, " ");
                md = atoi(read_point);
                read_point = strtok(NULL, " ");
                mx = atoi(read_point);
                read_point = strtok(NULL, " ");
                my = atoi(read_point);
                break;
            case TERM:
                md = 0;
                mx = 0;
                my = 0;
                break;
            case N_op:
                goto _error_read_file_ ;
                break;
        }
        memory[address].value.instruction.op = instruction;
        memory[address].value.instruction.md = md;
        memory[address].value.instruction.mx = mx;
        memory[address].value.instruction.my = my;
        memory[address].status = INUSE;
    }
    free(line);
    fclose(fp);
    return 0;

_error_read_file_:
    fprintf(stderr, "error:: #%d: Something wrong when reading file\n", address);
_error_file_open_:
    fclose(fp);
    return 1;
}


void excute_code()
{
    int curr = 0;
    int data;
    int target_address;
    int x_address;
    int y_address;

    while (curr < MEM_SIZE)
    {
        if (memory[curr].status == EMPTY) {
            curr++;
            continue;
        }
        Instruction* curr_instruction = &(memory[curr].value.instruction) ;
        switch (curr_instruction->op)
        {
            case READ:
                target_address = memory[curr].value.instruction.md;
                scanf("%d", &data);
                memory[target_address].value.instruction.op = N_op;
                memory[target_address].value.instruction.md = data;
                memory[target_address].status = INUSE;
                break;

            case PRINT:
                target_address = memory[curr].value.instruction.md;
                printf("%d\n", memory[target_address].value.integer);
                break;

            case ASSIGN:
                target_address = memory[curr].value.instruction.md;
                x_address = memory[curr].value.instruction.mx;
                data = memory[x_address].value.integer;
                memory[target_address].value.integer = data;
                memory[target_address].status = INUSE;
                break;

            case DEFINE:
                target_address = memory[curr].value.instruction.md;
                data = memory[curr].value.instruction.mx;
                memory[target_address].value.integer = data;
                memory[target_address].status = INUSE;
                break;
            case JUMP:
                target_address = memory[curr].md;
                if (memory[target_address].md != 0)
                    curr = memory[curr].mx - 1;
                break;
            case ADD:
                target_address = memory[curr].md;
                x_address = memory[curr].mx;
                y_address = memory[curr].my;
                memory[target_address].instruction = N_op;
                memory[target_address].md = memory[x_address].md + memory[y_address].md;
                memory[target_address].status = INUSE;
                break;
            case MINUS:
                target_address = memory[curr].md;
                x_address = memory[curr].mx;
                y_address = memory[curr].my;
                memory[target_address].instruction = N_op;
                memory[target_address].md = memory[x_address].md - memory[y_address].md;
                memory[target_address].status = INUSE;
                break;
            case MOD:
                target_address = memory[curr].md;
                x_address = memory[curr].mx;
                y_address = memory[curr].my;
                memory[target_address].instruction = N_op;
                memory[target_address].md = memory[x_address].md % memory[y_address].md;
                memory[target_address].status = INUSE;
                break;
            case EQ:
                target_address = memory[curr].md;
                x_address = memory[curr].mx;
                y_address = memory[curr].my;
                memory[target_address].instruction = N_op;
                memory[target_address].md = (memory[x_address].md == memory[y_address].md)?  1 : 0 ;
                memory[target_address].status = INUSE;
                break;
            case LESS:
                target_address = memory[curr].md;
                x_address = memory[curr].mx;
                y_address = memory[curr].my;
                memory[target_address].instruction = N_op;
                memory[target_address].md = (memory[x_address].md < memory[y_address].md)?  1 : 0 ;
                memory[target_address].status = INUSE;
                break;
            case TERM:
                return ;
            case N_op:
                break;
        }
        curr++;
    }
}

int main (int argc, char* argv[])
{
    if (argc == 1) { 
        printf("Please restart with input file..\n"); 
        printf("./<excution_file_name> <input_file_name>\n"); 
        return 0; 
    }
    if (read_code(argv[1])){
        fprintf(stderr, "\nFailed to compile your code...\n");
        return 0;
    }
    excute_code();

    return 0 ;
}
