#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FREE(p) free(p), p = NULL

typedef struct {
	int number;
	int rows;
	int row_slots;
	int columns1;
	int columns2;
	int column_slots;
	int cells_n;
	int blocks_n;
	char *cells;
	char *blocks;
	int *node_blocks;
	int *grid_blocks;
	int row_nodes_n;
}
piece_t;

typedef struct node_s node_t;

typedef struct {
	int grid_origin;
	piece_t *piece;
	node_t *column;
}
row_node_t;

struct node_s {
	union {
		int rows;
		row_node_t *row_node;
	};
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

char *read_piece(piece_t *, int);
char *rotate_piece(piece_t *, piece_t *);
char *flip_piece(piece_t *, piece_t *);
char *set_piece(piece_t *, int, int, int, int);
char *set_piece_blocks(piece_t *);
int compare_pieces(piece_t *, piece_t *);
void set_column_node(node_t *, node_t *);
void print_piece(piece_t *);
void set_piece_row_nodes(piece_t *);
void set_slot_row_nodes(piece_t *, int, int);
void set_row_node(int, piece_t *, int, node_t *);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(int);
void cover_column(node_t *);
void uncover_column(node_t *);
void add_piece(piece_t *, int);
void free_data(int);
void free_piece(piece_t *);

static char *grid_cells;
static int grid_rows, grid_columns1, grid_columns2, grid_cells_n1, cost, solutions;
static piece_t *pieces;
static row_node_t *row_nodes, **choices;
static node_t *nodes, **tops, *header, *row_node, *stop_column;

int main(void) {
	int grid_cells_n2, pieces_n, column_nodes_n1, column_nodes_n2, blocks_n, row_nodes_n, pieces_r, nodes_n, grid_stars, i;
	if (scanf("%d%d", &grid_rows, &grid_columns1) != 2 || grid_rows < 1 || grid_columns1 < 1) {
		fprintf(stderr, "Invalid grid size\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	while (getchar() != '\n');
	grid_cells_n1 = grid_rows*grid_columns1;
	grid_columns2 = grid_columns1+1;
	grid_cells_n2 = grid_rows*grid_columns2;
	grid_cells = malloc((size_t)(grid_cells_n2+1));
	if (!grid_cells) {
		fprintf(stderr, "Could not allocate memory for grid_cells\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (i = 0; i < grid_rows; i++) {
		int j;
		for (j = 0; j < grid_columns1; j++) {
			int c = getchar();
			if (c != '*' && c != '.') {
				fprintf(stderr, "Invalid grid data\n");
				fflush(stderr);
				free_data(0);
				return EXIT_FAILURE;
			}
			grid_cells[i*grid_columns2+j] = (char)c;
		}
		if (getchar() != '\n') {
			fprintf(stderr, "Invalid grid data\n");
			fflush(stderr);
			free_data(0);
			return EXIT_FAILURE;
		}
		grid_cells[i*grid_columns2+j] = '\n';
	}
	grid_cells[grid_cells_n2] = 0;
	if (scanf("%d", &pieces_n) != 1 || pieces_n < 1) {
		fprintf(stderr, "Invalid number of pieces\n");
		fflush(stderr);
		free_data(0);
		return EXIT_FAILURE;
	}
	pieces = malloc(sizeof(piece_t)*(size_t)(pieces_n*8));
	if (!pieces) {
		fprintf(stderr, "Could not allocate memory for pieces\n");
		fflush(stderr);
		free_data(0);
		return EXIT_FAILURE;
	}
	column_nodes_n1 = grid_cells_n1+pieces_n;
	column_nodes_n2 = column_nodes_n1+1;
	blocks_n = 0;
	row_nodes_n = 0;
	pieces_r = 0;
	for (i = 0; i < pieces_n; i++) {
		int piece_f, piece_l, j, r;
		if (!read_piece(pieces+pieces_r, i)) {
			free_data(pieces_r);
			return EXIT_FAILURE;
		}
		piece_f = pieces_r;
		blocks_n += pieces[pieces_r].blocks_n;
		row_nodes_n += pieces[pieces_r].row_nodes_n;
		pieces_r++;
		j = 1;
		do {
			int k;
			if (!rotate_piece(pieces+pieces_r-1, pieces+pieces_r)) {
				free_data(pieces_r);
				return EXIT_FAILURE;
			}
			r = compare_pieces(pieces+piece_f, pieces+pieces_r);
			for (k = piece_f+1; k < pieces_r && !r; k++) {
				r = compare_pieces(pieces+k, pieces+pieces_r);
			}
			if (!r) {
				row_nodes_n += pieces[pieces_r].row_nodes_n;
				pieces_r++;
				j++;
			}
			else {
				free_piece(pieces+pieces_r);
			}
		}
		while (j < 4 && !r);
		piece_l = pieces_r;
		j = piece_f;
		do {
			int k;
			if (!flip_piece(pieces+j, pieces+pieces_r)) {
				free_data(pieces_r);
				return EXIT_FAILURE;
			}
			r = compare_pieces(pieces+piece_f, pieces+pieces_r);
			for (k = piece_f+1; k < piece_l && !r; k++) {
				r = compare_pieces(pieces+k, pieces+pieces_r);
			}
			if (!r) {
				row_nodes_n += pieces[pieces_r].row_nodes_n;
				pieces_r++;
				j++;
			}
			else {
				free_piece(pieces+pieces_r);
			}
		}
		while (j < piece_l && !r);
	}
	row_nodes = malloc(sizeof(row_node_t)*(size_t)row_nodes_n);
	if (!row_nodes) {
		fprintf(stderr, "Could not allocate memory for row_nodes\n");
		fflush(stderr);
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	choices = malloc(sizeof(row_node_t *)*(size_t)pieces_n);
	if (!choices) {
		fprintf(stderr, "Could not allocate memory for choices\n");
		fflush(stderr);
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	nodes_n = column_nodes_n2+row_nodes_n;
	nodes = malloc(sizeof(node_t)*(size_t)nodes_n);
	if (!nodes) {
		fprintf(stderr, "Could not allocate memory for nodes\n");
		fflush(stderr);
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	for (i = column_nodes_n2; i < nodes_n; i++) {
		nodes[i].row_node = row_nodes+i-column_nodes_n2;
	}
	tops = malloc(sizeof(node_t *)*(size_t)column_nodes_n1);
	if (!tops) {
		fprintf(stderr, "Could not allocate memory for tops\n");
		fflush(stderr);
		free_data(pieces_r);
		return EXIT_FAILURE;
	}
	header = nodes+column_nodes_n1;
	set_column_node(nodes, header);
	for (i = 0; i < column_nodes_n1; i++) {
		set_column_node(nodes+i+1, nodes+i);
		tops[i] = nodes+i;
	}
	row_node = header+1;
	for (i = 0; i < pieces_r; i++) {
		print_piece(pieces+i);
		set_piece_row_nodes(pieces+i);
	}
	for (i = 0; i < column_nodes_n1; i++) {
		link_top(nodes+i, tops[i]);
	}
	grid_stars = 0;
	for (i = 0; i < grid_rows; i++) {
		int j;
		for (j = 0; j < grid_columns1; j++) {
			if (grid_cells[i*grid_columns2+j] == '*') {
				grid_stars++;
			}
			else {
				cover_column(nodes+i*grid_columns1+j);
			}
		}
	}
	if (blocks_n == grid_stars) {
		stop_column = header;
		dlx_search(0);
	}
	else if (blocks_n > grid_stars) {
		stop_column = nodes+grid_cells_n1;
		dlx_search(0);
	}
	printf("\nCost %d\nSolutions %d\n", cost, solutions);
	fflush(stdout);
	free_data(pieces_r);
	return EXIT_SUCCESS;
}

char *read_piece(piece_t *piece, int number) {
	int rows, columns, cell, i;
	if (scanf("%d%d", &rows, &columns) != 2 || rows < 1 || columns < 1) {
		fprintf(stderr, "Invalid piece size\n");
		fflush(stderr);
		return NULL;
	}
	while (getchar() != '\n');
	if (!set_piece(piece, number, rows, columns, 0)) {
		return NULL;
	}
	cell = 0;
	for (i = 0; i < piece->rows; i++) {
		int j = 0;
		do {
			int c = getchar();
			if (c == '\n') {
				break;
			}
			else if (isalnum(c)) {
				piece->blocks_n++;
			}
			else if (c != ' ') {
				fprintf(stderr, "Invalid piece data\n");
				fflush(stderr);
				FREE(piece->cells);
				return NULL;
			}
			piece->cells[cell++] = (char)c;
			j++;
		}
		while (j < piece->columns1);
		if (j < piece->columns1) {
			while (j < piece->columns1) {
				piece->cells[cell++] = ' ';
				j++;
			}
		}
		else {
			if (getchar() != '\n') {
				fprintf(stderr, "Invalid piece data\n");
				fflush(stderr);
				FREE(piece->cells);
				return NULL;
			}
		}
		piece->cells[cell++] = '\n';
	}
	if (piece->blocks_n < 1) {
		fprintf(stderr, "Invalid piece data\n");
		fflush(stderr);
		FREE(piece->cells);
		return NULL;
	}
	piece->cells[cell] = 0;
	if (!set_piece_blocks(piece)) {
		FREE(piece->cells);
	}
	return piece->cells;
}

char *rotate_piece(piece_t *piece, piece_t *rotated) {
	int i;
	if (!set_piece(rotated, piece->number, piece->columns1, piece->rows, piece->blocks_n)) {
		return NULL;
	}
	for (i = 0; i < rotated->rows; i++) {
		int j;
		for (j = 0; j < rotated->columns1; j++) {
			rotated->cells[i*rotated->columns2+j] = piece->cells[piece->cells_n-piece->columns2-j*piece->columns2+i];
		}
		rotated->cells[i*rotated->columns2+j] = '\n';
	}
	rotated->cells[rotated->cells_n] = 0;
	if (!set_piece_blocks(rotated)) {
		FREE(rotated->cells);
	}
	return rotated->cells;
}

char *flip_piece(piece_t *piece, piece_t *flipped) {
	int i;
	if (!set_piece(flipped, piece->number, piece->rows, piece->columns1, piece->blocks_n)) {
		return NULL;
	}
	for (i = 0; i < flipped->cells_n; i += flipped->columns2) {
		int j;
		for (j = 0; j < flipped->columns2; j++) {
			flipped->cells[i+j] = piece->cells[flipped->cells_n-flipped->columns2-i+j];
		}
	}
	flipped->cells[flipped->cells_n] = 0;
	if (!set_piece_blocks(flipped)) {
		FREE(flipped->cells);
	}
	return flipped->cells;
}

char *set_piece(piece_t *piece, int number, int rows, int columns, int blocks_n) {
	piece->number = number;
	piece->rows = rows;
	piece->row_slots = rows > grid_rows ? 0:grid_rows-rows+1;
	piece->columns1 = columns;
	piece->columns2 = columns+1;
	piece->column_slots = columns > grid_columns1 ? 0:grid_columns1-columns+1;
	piece->cells_n = rows*piece->columns2;
	piece->blocks_n = blocks_n;
	piece->cells = malloc((size_t)(piece->cells_n+1));
	if (!piece->cells) {
		fprintf(stderr, "Could not allocate memory for piece->cells\n");
		fflush(stderr);
	}
	return piece->cells;
}

char *set_piece_blocks(piece_t *piece) {
	int block, i;
	piece->blocks = malloc((size_t)piece->blocks_n);
	if (!piece->blocks) {
		fprintf(stderr, "Could not allocate memory for piece->blocks\n");
		fflush(stderr);
		return NULL;
	}
	piece->node_blocks = malloc(sizeof(int)*(size_t)piece->blocks_n);
	if (!piece->node_blocks) {
		fprintf(stderr, "Could not allocate memory for piece->node_blocks\n");
		fflush(stderr);
		FREE(piece->blocks);
		return NULL;
	}
	piece->grid_blocks = malloc(sizeof(int)*(size_t)piece->blocks_n);
	if (!piece->grid_blocks) {
		fprintf(stderr, "Could not allocate memory for piece->grid_blocks\n");
		fflush(stderr);
		FREE(piece->node_blocks);
		FREE(piece->blocks);
		return NULL;
	}
	block = 0;
	for (i = 0; i < piece->rows; i++) {
		int j;
		for (j = 0; j < piece->columns1; j++) {
			if (piece->cells[i*piece->columns2+j] != ' ') {
				piece->blocks[block] = piece->cells[i*piece->columns2+j];
				piece->node_blocks[block] = i*grid_columns1+j;
				piece->grid_blocks[block] = i*grid_columns2+j;
				block++;
			}
		}
	}
	piece->row_nodes_n = piece->row_slots*piece->column_slots*(1+piece->blocks_n);
	return piece->blocks;
}

int compare_pieces(piece_t *piece1, piece_t *piece2) {
	return piece2->rows == piece1->rows && piece2->columns1 == piece1->columns1 && !strcmp(piece2->cells, piece1->cells);
}

void set_column_node(node_t *node, node_t *left) {
	node->rows = 0;
	link_left(node, left);
}

void print_piece(piece_t *piece) {
	printf("\nPiece %d\nSize %dx%d\n%s", piece->number+1, piece->rows, piece->columns1, piece->cells);
}

void set_piece_row_nodes(piece_t *piece) {
	int i;
	for (i = 0; i < piece->row_slots; i++) {
		int j;
		for (j = 0; j < piece->column_slots; j++) {
			set_slot_row_nodes(piece, i*grid_columns1+j, i*grid_columns2+j);
		}
	}
}

void set_slot_row_nodes(piece_t *piece, int node_origin, int grid_origin) {
	int i;
	set_row_node(grid_origin, piece, node_origin+piece->node_blocks[0], row_node+piece->blocks_n);
	for (i = 1; i < piece->blocks_n; i++) {
		set_row_node(grid_origin, piece, node_origin+piece->node_blocks[i], row_node-1);
	}
	set_row_node(grid_origin, piece, grid_cells_n1+piece->number, row_node-1);
}

void set_row_node(int grid_origin, piece_t *piece, int column, node_t *left) {
	row_node->row_node->grid_origin = grid_origin;
	row_node->row_node->piece = piece;
	row_node->row_node->column = nodes+column;
	link_left(row_node, left);
	link_top(row_node, tops[column]);
	tops[column] = row_node++;
	nodes[column].rows++;
}

void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

void dlx_search(int depth) {
	cost++;
	if (header->right < stop_column) {
		node_t *column_min = header->right, *column, *row;
		for (column = column_min->right; column < stop_column; column = column->right) {
			if (column->rows < column_min->rows) {
				column_min = column;
			}
		}
		cover_column(column_min);
		for (row = column_min->bottom; row != column_min; row = row->bottom) {
			node_t *node;
			for (node = row->right; node != row; node = node->right) {
				cover_column(node->row_node->column);
			}
			choices[depth] = row->row_node;
			dlx_search(depth+1);
			for (node = row->left; node != row; node = node->left) {
				uncover_column(node->row_node->column);
			}
		}
		uncover_column(column_min);
	}
	else {
		solutions++;
		if (solutions == 1) {
			int i;
			for (i = 0; i < depth; i++) {
				add_piece(choices[i]->piece, choices[i]->grid_origin);
			}
			printf("\nCost %d\n\n%s", cost, grid_cells);
			fflush(stdout);
		}
	}
}

void cover_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			node->row_node->column->rows--;
			node->bottom->top = node->top;
			node->top->bottom = node->bottom;
		}
	}
}

void uncover_column(node_t *column) {
	node_t *row;
	for (row = column->top; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			node->top->bottom = node;
			node->bottom->top = node;
			node->row_node->column->rows++;
		}
	}
	column->left->right = column;
	column->right->left = column;
}

void add_piece(piece_t *piece, int grid_origin) {
	int i;
	for (i = 0; i < piece->blocks_n; i++) {
		grid_cells[grid_origin+piece->grid_blocks[i]] = piece->blocks[i];
	}
}

void free_data(int pieces_r) {
	if (tops) {
		FREE(tops);
	}
	if (nodes) {
		FREE(nodes);
	}
	if (choices) {
		FREE(choices);
	}
	if (row_nodes) {
		FREE(row_nodes);
	}
	if (pieces) {
		int i;
		for (i = 0; i < pieces_r; i++) {
			free_piece(pieces+i);
		}
		FREE(pieces);
	}
	if (grid_cells) {
		FREE(grid_cells);
	}
}

void free_piece(piece_t *piece) {
	FREE(piece->grid_blocks);
	FREE(piece->node_blocks);
	FREE(piece->blocks);
	FREE(piece->cells);
}
