#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define ASCII_SIZE		257
#define EOF_NUM			ASCII_SIZE - 1

typedef enum {
	archive,
	list,
	extract,
	compress,
	decompress,
	N_md
} Modetype;

typedef enum {
	LTRT,
	LTRN,
	LNRT,
	LNRN,
	LEOF,
	REOF,
} TreeNodeState ;

char* md_str[5] = {
	"archive",
	"list",
	"extract",
	"compress",
	"decompress",
};

typedef struct {
	int byte ;
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

	if (root->data.byte == LTRT) {
		if (root->left->data.byte == EOF_NUM)
			root->data.byte = LEOF ;
		else if (root->right->data.byte == EOF_NUM)
			root->data.byte = REOF ;
	}
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
	unsigned char byte = (unsigned char) root->data.byte ;
	fwrite(&(byte), sizeof(unsigned char), 1, fp) ;
	if (root->left != NULL)
		write_tree(root->left, fp) ;
	if (root->right != NULL) 
		write_tree(root->right, fp) ;
}

void huffman_encode_head(TreeNode* root, FILE* fp)
{
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

unsigned char get_char_from_buffer(WBuffer* buffer) 
{
	unsigned char byte = 0 ;

	for (int i = 0 ; i < 8 ; i ++)
	{
		// if (buffer->front == buffer->rear){
		// 	fprintf(stderr, "Failed to encoding.. => %d\n", byte);
		// 	break ;
		// }
		byte += buffer->memory[buffer->front] << i ;
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
	do {
		if ((data = fgetc(pFile_r)) == EOF) 
			data = EOF_NUM ;
		
		for (int i = 0 ; i < strlen(huffman_string[data]); i++) {
			push_buffer(huffman_string[data][i], buffer_w) ;
		}
		int bit_far = (buffer_w->rear > buffer_w->front)? buffer_w->rear - (buffer_w->front+8)%buffer_w->mod 
														: buffer_w->mod - (buffer_w->front-buffer_w->rear) ;
		while (bit_far > 8) {
			unsigned char ch = get_char_from_buffer(buffer_w) ;
			fputc(ch, pFile_w) ;
			bit_far -= 8 ;
		}
	} while (data != EOF_NUM) ;

	if (!is_buffer_empty(buffer_w)) {
		unsigned char ch = get_char_from_buffer(buffer_w) ;
		fputc(ch, pFile_w) ;
	}

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

	for (int i = 0 ; i < ASCII_SIZE; i++) {
		if (char_freq[i] != 0) {
			Data data = {i, char_freq[i]} ;
			TreeNode* new_tree = singleton_tree(data) ;
			push_pqueue(Q, new_tree) ;
		}
	}
	// add terminal character
	Data data = {EOF_NUM, 1} ;
	TreeNode* new_tree = singleton_tree(data) ;
	push_pqueue(Q, new_tree) ;
	char_freq[EOF_NUM] = 1 ;

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
	
	data.byte = (int) buffer ;
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
	case LEOF:
		left_child = decompress_file_head(fp, 1) ;
		right_child = decompress_file_head(fp, 1) ;
		left_child->data.byte = EOF_NUM ;
		break;
	case REOF:
		left_child = decompress_file_head(fp, 1) ;
		right_child = decompress_file_head(fp, 1) ;
		right_child->data.byte = EOF_NUM ;
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
	char extension[50] = "";

	char* ptr = strrchr(filename, '.') ;
	*ptr = '\0' ;
	ptr = strrchr(filename, '.') ;
	if (ptr != NULL) {
		strcpy(extension, ptr) ;
		ptr = '\0' ;
	}
	strcat(filename, "_cp") ;
	strcat(filename, extension) ;
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
				if (curr_node->data.byte == EOF_NUM) {
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
	TreeNode* root = decompress_file_head(fp, 0) ;
	decompress_file_body(fp, root, filename) ;
}


Modetype get_md_type (char* mode)
{
	int i;
	for (i = 0; i < N_md; i++) {
		if (strcmp(mode, md_str[i]) == 0)
			return i;
	}
	return N_md;
}

void write_archive_file (FILE* pFile_w, char curr_dir[], char target_file[], int parsing_point)
{
	FILE* pFile_r ;
	char filepath_on_tar[400] = "./" ;
	char filepath[400] ;
	char* buffer ;
	size_t result ;
	struct stat sfile ;
	unsigned int filepath_len ;
	unsigned int size_of_file ;
	int kind ;

	strcpy(filepath, curr_dir) ;
	if (filepath[strlen(filepath)-1] != '/')
		strcat(filepath, "/") ;
	strcat(filepath, target_file) ;
	strcat(filepath_on_tar, &filepath[parsing_point]) ;

	if (stat(filepath, &sfile) == -1) {
		fprintf(stderr, "Error occurred\n") ;
	}

	printf("filepath: %s\n", filepath_on_tar) ;

	kind = sfile.st_mode & S_IFMT ;
	filepath_len = (unsigned int)strlen(filepath_on_tar) ;
	size_of_file = (kind != S_IFDIR)? (unsigned int)sfile.st_size : 0 ;

	// write the head.
	fwrite(&filepath_len, sizeof(unsigned int), 1, pFile_w) ;
	fwrite(filepath_on_tar, sizeof(char), filepath_len, pFile_w) ;
	fwrite(&(sfile.st_mode), sizeof(mode_t), 1, pFile_w) ;
	fwrite(&size_of_file, sizeof(unsigned int), 1, pFile_w) ;

	// write the body.
	pFile_r = fopen(filepath, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_read_ ;
	}

	buffer = (char*) malloc(sizeof(char)*size_of_file) ;
	if (buffer == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_read_ ;
	}
	result = fread(buffer, sizeof(char), size_of_file, pFile_r) ;
	fwrite(buffer, sizeof(char), size_of_file, pFile_w) ;
	free(buffer) ;

	if (kind == S_IFDIR) {
		DIR* dp ;
		struct dirent* ep ;

		dp = opendir(filepath) ;
		if (dp != NULL) {
			while (ep = readdir(dp)){
				if ((strcmp(".", ep->d_name) == 0) || (strcmp("..", ep->d_name) == 0))
					continue ;
				write_archive_file(pFile_w, filepath, ep->d_name, parsing_point) ;
			}
			(void) closedir(dp) ;
		}
		else 
			perror("Couldn't open the directory") ;
	}


_error_file_read_:
	if (pFile_r != NULL)
		fclose(pFile_r) ;
}

void archive_mode(char ar_filename[], char target_path[])
{
	FILE* pFile_w ;
	struct stat sfile ;
	char curr_dir[400] ;
	char filename[100] ;
	int i ; 

	for (i = strlen(target_path)-1 ; i >= 0 ; i--) {
		if (target_path[i-1] == '/') {
			strcpy(filename, &target_path[i]) ;
			strncpy(curr_dir, target_path, i) ;
			curr_dir[i] = '\0' ;
			break ;
		}
	}

	pFile_w = fopen(ar_filename, "wb") ; 
	if (pFile_w == NULL) {
		fprintf(stderr, "Error occurred\n") ;
		return ;
	}
	

	if (stat(target_path, &sfile) == -1) {
		fprintf(stderr, "Error occurred\n") ;
		goto _error_stat_call_ ;
	}
	
	int kind = sfile.st_mode & S_IFMT ;
	switch (kind)
	{
		case S_IFDIR:
			write_archive_file(pFile_w, curr_dir, filename, i) ;
			break ;
		
		default:
			fprintf(stderr, "This is not a directory\n") ;
			break ;
	}
	
_error_stat_call_:
	fclose(pFile_w) ;
}

void list_mode(char ar_filename[]) 
{
	FILE* pFile_r ;
	long lSize ;
	char * filename = NULL ;
	unsigned int n ;
	unsigned int m ;
	mode_t file_mode ;
	size_t result ;
	int i, count ;

	pFile_r = fopen(ar_filename, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		exit(1) ;
	}

	while (1)
	{
		result = fread(&n, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			if (feof(pFile_r))
				break ;
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		filename = (char*)malloc(sizeof(char) * (n+1)) ;
		result = fread(filename, sizeof(char), n, pFile_r) ;
		if (result != n) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		filename[n] = '\0' ;
		result = fread(&file_mode, sizeof(mode_t), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		result = fread(&m, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_read_file_ ;
		}
		
		if (fseek(pFile_r, m, SEEK_CUR))
			fputs("Reading seek error", stderr) ;

		count = -1 ;
		for (i = 0 ; i < strlen(filename) ; i++) {
			if (filename[i] == '/')
				count++ ;
		}
		for (i = strlen(filename)-1 ; i >= 0 ; i--) {
			if (filename[i-1] == '/') {
				while (count--)
					putchar('\t') ;
				if ((file_mode & S_IFMT) == S_IFDIR)
					printf("|___%s(D)\n", &filename[i]) ;
				else
					printf("|___%s\n", &filename[i]) ;
				break;
			}
		}
		// printf("filename: %s\n", filename) ;

		free(filename) ;
	}

_error_read_file_:
	fclose(pFile_r) ;
}

void extract_file (FILE* pFile_r, char filepath[], int file_size, int file_mode)
{
	FILE* pFile_w ;
	char* buffer ;
	size_t result ;

	switch (file_mode & S_IFMT)
	{
		case S_IFDIR:
			if (mkdir(filepath, 0775) == -1) {
				fprintf(stderr, "Fail to make directory: %s\n", filepath) ;
				return ;
			}
			break ;
		
		default:
			printf("%s\n", filepath) ;
			printf("sizse:%d\n", file_size) ;
			pFile_w = fopen(filepath, "wb") ;
			if (pFile_w == NULL) {
				fputs("File error", stderr) ;
			}
			buffer = (char*) malloc(sizeof(char)*file_size) ;
			if (buffer == NULL) {
				fputs("Allocation error", stderr) ;
				break ;
			}
			result = fread(buffer, 1, file_size, pFile_r) ;
		
			if (result != file_size) {
				fputs("Reading error", stderr) ;
				free(buffer) ;
				break ;
			}
			fwrite(buffer, sizeof(char), file_size, pFile_w) ;
			if (chmod(filepath, file_mode) == -1)
				fprintf(stderr, "chmod is failed..\n") ;

			fclose(pFile_w) ;
			free(buffer) ;
			break ;
	}
}

void extract_mode(char ar_filename[]) 
{
	FILE* pFile_r ;
	long lSize ;
	char * filename = NULL ;
	unsigned int n ;
	unsigned int m ;
	mode_t file_mode ;
	size_t result ;

	pFile_r = fopen(ar_filename, "rb") ;
	if (pFile_r == NULL) {
		fputs("File error", stderr) ;
		goto _error_file_open_ ;
	}
	while (1)
	{
		result = fread(&n, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			if (feof(pFile_r))
				break ;
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		filename = (char*)malloc(sizeof(char) * (n+1)) ;
		result = fread(filename, sizeof(char), n, pFile_r) ;
		if (result != n) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		filename[n] = '\0' ;
		result = fread(&file_mode, sizeof(mode_t), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		result = fread(&m, sizeof(unsigned int), 1, pFile_r) ;
		if (result != 1) {
			fputs("Reading error", stderr) ;
			goto _error_file_open_ ;
		}
		

		int kind = file_mode & S_IFMT ;
		extract_file(pFile_r, filename, m, file_mode) ;

		
		free(filename) ;
	}

_error_file_open_:
	if (pFile_r == NULL)
		fclose(pFile_r) ;
}


void print_menu()
{
    puts("=== ARCHIVE & HUFFMAN COMPRESS PROGRAM ===") ;
	puts("Command-line Interface");
	puts("(1) star archive <archive-file-name> <target-directory-path>") ;
	puts("(2) star list <archive-file-name>") ;
	puts("(3) star extract <archive-file-name>") ;
	puts("(4) star compress <target-file-name>") ;
	puts("(5) star decompress <copressed-file-name>") ;
}

int main ()
{
	char star[16], mode[16] ;
	char filename[200] ;
	char target_path[200] ;
	char temp[200] ;

	print_menu() ;
	scanf("%s %s", star, mode) ;
	if (strcmp(star, "star") != 0) {
		fprintf(stderr, "wrong command\n") ;
		return 0 ;
	}
	switch (get_md_type(mode))
	{
		case archive:
			scanf("%s", filename) ;
			scanf("%s", target_path) ;
			if (target_path[0] != '.') {
				strcpy(temp, "./") ;
				strcat(temp, target_path) ;
				strcpy(target_path, temp) ;
			}
			printf("archive\n");
			archive_mode(filename, target_path) ;
			break ;
		case list:
			scanf("%s", filename) ;
			list_mode(filename) ;
			break ;
		case extract:
			scanf("%s", filename) ;
			extract_mode(filename) ;
			break;
		case compress:
			scanf("%s", filename) ;
			compress_file(filename) ;
			break;
		case decompress:
			scanf("%s", filename) ;
			decompress_file(filename) ;
			break;
		case N_md:
			fprintf(stderr, "wrong archive mode\n") ;
			break;
	}

	return 0;
}
