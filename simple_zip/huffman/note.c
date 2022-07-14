#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASCII_SIZE		256	

// L:left, R:right, T:terminal, N:non-terminal
// EOF character을 받을 수 있는 TreeNodeState를 하나 받아야한다. 
// 5번, 6번 추가하는 방식.

// tar 파일을 만들어라. 그리고 그거가지고 잘 파일 푸는 게 가능한가?
// zip 한것도 서로 모아서, 걔가 잘 출력이 되는 가?
// compress , decompress
typedef enum {
	LTRT,
	LTRN,
	LNRT,
	LNRN,
} TreeNodeState ;

typedef enum {
	compress = 1,
	decompress,
} Menu ;

typedef struct {
	unsigned char byte ;
	int freq ;
} Data ;

typedef struct tree_node {
	Data data ;
	struct tree_node* left ;
	struct tree_node* right ;
} TreeNode ;

typedef struct priority_queue_node {
	TreeNode* btree ;
	struct priority_queue_node* next ;
	int count ;
} PQnode ;

typedef struct priority_queue{
	PQnode* head ;
	int count ;
} PQueue ;

long initial_size = 0;
long total_bits = 0;
char* huffman_string[ASCII_SIZE] = {NULL};
unsigned char _EOF = 0 ;
typedef struct write_buffer {
	unsigned char* memory ;
	int front ;
	int rear ;
	int mod ;
} WBuffer ;

///// Tree /////
TreeNode* singleton_tree (Data data)
{
	TreeNode* new_node = malloc(sizeof(TreeNode)) ;
	new_node->data = data ;
	new_node->left = NULL ;
	new_node->right = NULL ;

	return new_node ;
}

void delete_tree(TreeNode* root)
{
	if (root->left != NULL)
		delete_tree(root->left) ;
	if (root->right != NULL)
		delete_tree(root->right) ;
	
	free(root) ;
}

int is_terminal(TreeNode* node)
{
	return (node->left == NULL && node->right == NULL)?  1 : 0 ;
}

///// Queue /////

PQueue* create_queue()
{
	PQueue* return_queue = malloc(sizeof(PQueue)) ;
	return_queue->head = NULL ;
	return_queue->count = 0 ;

	return return_queue ;
} 

int is_empty(PQueue* Q) 
{
   return (Q->count == 0);
}

TreeNode* pop_pqueue(PQueue* Q) {
	TreeNode* return_data = Q->head->btree ;
	PQnode* temp = Q->head;
	Q->head = Q->head->next;
	Q->count-- ;
	free(temp);

	return return_data ;
}

void push_pqueue(PQueue* Q, TreeNode* new_tree)
{
	PQnode* new_node = malloc(sizeof(PQnode)) ;
	new_node->btree = new_tree ;
	new_node->next = NULL ;
	if (Q->head == NULL) {
		Q->head = new_node ;
		Q->count = 1 ;
		return ;
	}
	
	if (Q->head->btree->data.freq > new_tree->data.freq) {
		new_node->next = Q->head ;
		Q->head = new_node ;
	}
	else {
		PQnode* start = Q->head ;
		while (start->next != NULL && 
		start->next->btree->data.freq < new_tree->data.freq) {
			start = start->next ;
		}
		new_node->next = start->next ;
		start->next = new_node ;
	}
	Q->count++ ;
}

///// Huffman /////
void set_huffman_bits(TreeNode* root, char bits[], int bits_len)
{
	char temp[32];
	if(root->left == NULL && root->right == NULL) {
		// printf("%c[%d] / count:%d ->%s\n", root->data.byte, root->data.byte, root->data.freq, bits) ;
		huffman_string[root->data.byte] = (char*) malloc(sizeof(char) * (bits_len + 1)) ;
		strcpy(huffman_string[root->data.byte], bits) ;
		total_bits += root->data.freq * strlen(bits) ;
		return ;
	}	
	strcpy(temp, bits);
	strcat(temp, "1");
	set_huffman_bits(root->right, temp, bits_len+1);
	temp[bits_len] = '0';
	set_huffman_bits(root->left, temp, bits_len+1);
}

void count_ascii_freq(int char_freq[], char filename[])
{
	FILE* fp = NULL ;
	unsigned char * buffer = NULL ;
	long lSize ;
	size_t result ;

	fp = fopen(filename, "rb") ;
	if (fp == NULL) {
		fprintf(stderr, "file open is failed\n") ;
		goto _error_file_read_ ;
	}

	fseek(fp, 0, SEEK_END) ;
	lSize = ftell(fp) ;
	rewind(fp) ;

	initial_size = lSize * 8;
	buffer = (unsigned char*) malloc(sizeof(unsigned char)*lSize);
	if (buffer == NULL) {
		fputs("Reading error", stderr);
		goto _error_file_read_ ;
	}

	result = fread(buffer, 1, lSize, fp) ;
	if (result != lSize) {
		fputs("Reading error", stderr);
		goto _error_file_read_ ;
	}

	for (int i = 0 ; i < lSize ; i++) 
		char_freq[buffer[i]]++;

_error_file_read_:
	if (fp != NULL)
		fclose(fp) ;
	if (buffer != NULL)
		free(buffer) ;
}

void set_nonterminal_struct(TreeNode* root) 
{
	// (right terminal?)(left terminal?)(right child?)(left child?)
	// ex 1010 => root has right child which is a terminal.
	if (is_terminal(root))
		return ;

	if (is_terminal(root->left) && is_terminal(root->right))
		root->data.byte = LTRT ;
	else if (is_terminal(root->left) && !is_terminal(root->right))
		root->data.byte = LTRN ;
	else if (!is_terminal(root->left) && is_terminal(root->right))
		root->data.byte = LNRT ;
	else if (!is_terminal(root->left) && !is_terminal(root->right))
		root->data.byte = LNRN ;

	set_nonterminal_struct(root->left) ;
	set_nonterminal_struct(root->right) ;
}

TreeNode* search_huffman_tree(TreeNode* node, int bit)
{
	return (bit == 0)? node->left : node->right ;
}

///// file write /////
void write_tree(TreeNode* root, FILE* fp)
{
	
	fwrite(&(root->data.byte), sizeof(unsigned char), 1, fp) ;
	if (root->left != NULL)
		write_tree(root->left, fp) ;
	if (root->right != NULL) 
		write_tree(root->right, fp) ;
}

void huffman_encode_head(TreeNode* root, FILE* fp)
{
	fputc(_EOF, fp) ;
	set_nonterminal_struct(root) ;
	write_tree(root, fp);
}



////// BUFFER //////
WBuffer* buffer_init()
{
	WBuffer* new_buffer = malloc(sizeof(WBuffer)) ;
	new_buffer->mod = 200 ;
	new_buffer->front = 0 ;
	new_buffer->rear = 0 ;
	new_buffer->memory = malloc(sizeof(unsigned char) * 200) ;

	return new_buffer ;
}

int is_buffer_empty(WBuffer* buffer)
{
	return (buffer->front == buffer->rear) ;
}

int get_bit_from_buffer(WBuffer* buffer)
{
	int return_value = buffer->memory[buffer->front] ;
	buffer->front = (buffer->front + 1) % buffer->mod ;
	return return_value ;
}

unsigned char buffer_flush(WBuffer* buffer) 
{
	unsigned char byte = 0 ;
	while (buffer->front != buffer->rear)
	{
		int distance = (buffer->front+8)%buffer->mod - buffer->rear ;
		byte += buffer->memory[buffer->front] << distance ;
		buffer->front = (buffer->front + 1) % buffer->mod ;
	}
	return byte ;
}

int push_buffer(char bit, WBuffer* buffer) 
{
	buffer->memory[buffer->rear] = (bit == '1')?  1 : 0 ;
	buffer->rear = (buffer->rear + 1)% buffer->mod ;
}

void huffman_encode_body (FILE* pFile_w, char filename[]) 
{
	printf("filename:%s\n", filename) ;
	FILE* pFile_r = fopen(filename, "rb") ;
	int data ;
	if (pFile_r == NULL) {
		fprintf(stderr, "Failed to open file\n") ;
		return ;
	}

	WBuffer* buffer_w = buffer_init() ;
	while((data = fgetc(pFile_r)) != EOF)
	{
		for (int i = 0 ; i < strlen(huffman_string[data]); i++) {
			push_buffer(huffman_string[data][i], buffer_w) ;
			if (((buffer_w->front + 8) % buffer_w->mod) == buffer_w->rear) {
				unsigned char ch = buffer_flush(buffer_w) ;
				fputc(ch, pFile_w) ;
			}
		}
	}
	buffer_flush(buffer_w) ;

	free(buffer_w) ;
	fclose(pFile_r) ;
}

void compress_file(char filename[]) 
{
    FILE* fp = NULL ;
	int char_freq[ASCII_SIZE] = {0} ;
	PQueue* Q = create_queue() ;
	TreeNode* t1, * t2;
	
	count_ascii_freq(char_freq, filename);
	// char_freq[ASCII_SIZE-1] = 1 ;  // psedo-EOF
	// 	for (int i = 0 ; i < ASCII_SIZE ; i++) {
	// 		if (char_freq[i] == 0) {
	// 			_EOF = i ;
	// 			char_freq[i] = 1 ;
	// 			break ;
	// 		}
	// 	}

	char_freq[ASCII_SIZE-1] = 1 ;
	for (int i = 0 ; i < ASCII_SIZE; i++) {
		if (char_freq[i] != 0) {
			Data data = {i, char_freq[i]} ;
			TreeNode* new_tree = singleton_tree(data) ;
			push_pqueue(Q, new_tree) ;
		}
	}

	printf("\nQ->count:%d\n" ,Q->count);
	while(1)
	{
		if (is_empty(Q)) {
			fprintf(stderr, "error\n");
			return ;
		}
		t1 = pop_pqueue(Q) ;
		if (is_empty(Q)) {
			break;
		} 
		t2 = pop_pqueue(Q) ;

		TreeNode* new_tree = malloc(sizeof(TreeNode)) ;
		new_tree->data.freq = t1->data.freq + t2->data.freq ;
		new_tree->left = t1 ;
		new_tree->right = t2 ;
		push_pqueue(Q, new_tree) ;
	}

	printf("rood fre: %d\n", t1->data.freq) ;

	set_huffman_bits(t1, "", 0);

	for (int i = 0 ; i < ASCII_SIZE; i++) {
		if (char_freq[i] != 0) {
			printf("(%d) %10d: %s\n", i, char_freq[i], huffman_string[i]);
		}
	}

	printf("Before huffman, file size is %ld\n", initial_size);
	printf("total bits is %ld\n", total_bits);


	fp = fopen(strcat(filename, ".huffman"), "wb") ;

	char* ptr = strrchr(filename, '.') ;
	*ptr = '\0' ;

	huffman_encode_head(t1, fp);
	huffman_encode_body(fp, filename) ;

	fclose(fp) ;
	
	delete_tree(t1) ;
}

TreeNode* decompress_file_head(FILE* fp, int is_terminal) 
{
	unsigned char buffer ;
	fscanf(fp, "%c", &buffer) ;
	TreeNode* curr_node ;
	TreeNode* left_child ;
	TreeNode* right_child ;
	Data data ;
	
	data.byte = buffer ;
	curr_node = singleton_tree(data) ;
	if (is_terminal)
		return curr_node ;

	switch (buffer)
	{
	case LTRT:
		left_child = decompress_file_head(fp, 1) ;
		right_child = decompress_file_head(fp, 1) ;
		break;
	case LTRN:
		left_child = decompress_file_head(fp, 1) ;
		right_child = decompress_file_head(fp, 0) ;
		break;
	case LNRT:
		left_child = decompress_file_head(fp, 0) ;
		right_child = decompress_file_head(fp, 1) ;
		break;
	case LNRN:
		left_child = decompress_file_head(fp, 0) ;
		right_child = decompress_file_head(fp, 0) ;
		break;
	}
	curr_node->left = left_child ;
	curr_node->right = right_child ;

	return curr_node ;
}

void decompress_file_body (FILE* pFile_r, TreeNode* root, char filename[]) 
{
	WBuffer* buffer = buffer_init() ;
	FILE* pFile_w ;
	TreeNode* curr_node = root ;

	char* ptr = strrchr(filename, '.') ;
	*ptr = '\0' ;
	strcat(filename, "_copied") ;
	printf("write filename: %s\n", filename);
	pFile_w = fopen(filename, "wb") ;
	
	while (1)
	{
		if (is_buffer_empty(buffer)) {
			int data = fgetc(pFile_r);
			if (data == EOF) {
				fprintf(stderr, "error when reading file\n") ;
				break ;
			}
			for (int i = 0 ; i < 8 ; i++) {
				if (data & 1)
					push_buffer('1', buffer) ;
				else	
					push_buffer('0', buffer) ;
				data = data >> 1 ;
			}
		} else {
			int bit = get_bit_from_buffer(buffer) ;
			curr_node = search_huffman_tree(curr_node, bit) ;
			if (is_terminal(curr_node)) {
				if (curr_node->data.byte == _EOF) {
					break ;
				}
				fputc(curr_node->data.byte, pFile_w) ;
				curr_node = root ;
			}
		}
	}
	fclose(pFile_w) ;
}

void decompress_file(char filename[])
{
	unsigned char buffer ;
	FILE* fp = fopen(filename, "rb") ;
	_EOF = fgetc(fp) ;
	printf("decompressed EOF: %d\n", _EOF) ;
	TreeNode* root = decompress_file_head(fp, 0) ;
	decompress_file_body(fp, root, filename) ;

	set_huffman_bits(root, "", 0) ;
	for (int i = 0 ; i < ASCII_SIZE ; i ++) {
		if (huffman_string[i] != NULL) {
			printf("%d: %s\n", i, huffman_string[i]);
		}
	}
}

void print_menu()
{
    printf("=== HUFFMAN COMPRESS PROGRAM ===\n") ;
    printf("Which mode do you want?\n");
    printf("(1)compress  (2)decompress\n");
    printf("> ") ;
}

int main()
{
    int menu ;
    char filename[100] ;
    print_menu() ;
    scanf("%d", &menu) ;

    switch (menu)
    {
    case compress:
        printf("Enter the target file > ");
        scanf("%s", filename) ;
        compress_file(filename) ;
        break;
    
    case decompress:
		printf("Enter the target file > ");
		scanf("%s", filename) ;
		
        decompress_file(filename) ;
        break;
    }


    return 0;
}
