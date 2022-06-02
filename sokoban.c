#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define RATE 2 /* Rate of dynamic structures (e.g. board) growth. */
#define EMPTY_SQUARE '-'
#define EMPTY_TARGET_SQUARE '+'
#define WALL_SQUARE '#'
#define PLAYER_SQUARE '@'
#define PLAYER_TARGET_SQUARE '*'
#define EMPTY_SQUARE_SEEN '!'
#define EMPTY_TARGET_SQUARE_SEEN '%'
#define PLAYER_SQUARE_SEEN '^'
#define PLAYER_TARGET_SQUARE_SEEN '&'
#define LETTER_COUNT 'z' - 'a' + 1
#define DOWN 2
#define LEFT 4
#define RIGHT 6
#define UP 8

/* Position on board struct. */
typedef struct {
    int x;
    int y;
} tposition;

typedef struct {
    tposition box_positions[LETTER_COUNT];
    tposition player_position;
    char **table;
    int height;
    int hcapacity;
    int *widths;
} tboard;

typedef struct {
    char *items;
    int size;
    int capacity;
} trow;

/* Move struct.
   player_position - position before move,
   moved_box - moved box number */
typedef struct {
    tposition player_position;
    int moved_box;
} tmove;

typedef struct {
    tmove *items;
    int top;
    int capacity;
} tmove_stack;

void init_board(tboard *board);
void read_board(tboard *board);
void print_board(tboard *board);
void read_instructions(tboard *board);
void clear_board(tboard *board);
void init_row(trow *row);
void add_row(tboard *board, trow *row);
void add_square(trow *row, char c);
bool is_lowercase(char c);
bool is_uppercase(char c);
void init(tmove_stack *move_stack);
bool empty(tmove_stack *move_stack);
void pop(tmove_stack *move_stack, tmove *move);
void reverse_move(tboard *board, tmove move);
void make_move(tboard *board, tmove_stack *move_stack, int box, int direction);
void clear(tmove_stack *move_stack);
void update(tboard *board, tposition p, char c);
bool is_avaliable(tboard *board, int x, int y);
bool is_path(tboard *board, int x1, int y1, int x2, int y2);
void reset(tboard *board);
void push(tmove_stack *move_stack, tmove move);
bool is_target(tboard *board, tposition p);

int main(void) {
    tboard *board = malloc(sizeof(*board));
    init_board(board);
    read_board(board);
    print_board(board);
    read_instructions(board);
    clear_board(board);
    return 0;
}

void init_board(tboard *board) {
    board->table = NULL;
    board->widths = NULL;
    board->height = 0;
    board->hcapacity = 0;
}

void read_board(tboard *board) {
    char actual;
    char next;
    scanf("%c", &actual);
    scanf("%c", &next);
    trow *row = malloc(sizeof(*row));
    init_row(row);
    /* The board is always followed by 2 newline characters. */
    while (!(actual == '\n' && next == '\n')) {
        if (actual == '\n') {
            add_row(board, row);
            init_row(row);
        } else {
            tposition position;
            position.x = row->size;
            position.y = board->height;
            add_square(row, actual);
            if (is_lowercase(actual)) {
                board->box_positions[actual - 'a'] = position;
            } else if (is_uppercase(actual)) {
                board->box_positions[actual - 'A'] = position;
            } else if (actual == '@' || actual == '*') {
                board->player_position = position;
            }
        }
        actual = next;
        scanf("%c", &next);
    }
    add_row(board, row);
    free(row);
}

void print_board(tboard *board) {
    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->widths[y]; ++x) {
            printf("%c", board->table[y][x]);
        }
        printf("\n");
    }
}

/* Reads and follows instructions for the board. */
void read_instructions(tboard *board) {
    tmove_stack *move_stack = malloc(sizeof(*move_stack));
    init(move_stack);
    char c1;
    char c2;
    scanf("%c", &c1);
    while (c1 != '.') {
        if (c1 == '0' && !empty(move_stack)) {
            /* Reverse the move. */
            tmove move;
            pop(move_stack, &move);
            reverse_move(board, move);
        } else if (c1 != '0') {
            scanf("%c", &c2);
            make_move(board, move_stack, c1 - 'a', c2 - '0');
        }
        print_board(board);
        scanf("%c", &c1);
        scanf("%c", &c1);
    }
    clear(move_stack);
}

/* Free memory allocated for the board. */
void clear_board(tboard *board) {
    for (int i = 0; i < board->height; ++i) {
        free(board->table[i]);
    }
    free(board->table);
    free(board->widths);
    free(board);
}

void init_row(trow *row) {
    row->items = NULL;
    row->size = 0;
    row->capacity = 0;
}

void add_row(tboard *board, trow *row) {
    if (board->height == board->hcapacity) {
        board->hcapacity = RATE * board->hcapacity + 1;
        board->table = realloc(board->table, board->hcapacity * sizeof(*board->table));
        board->widths = realloc(board->widths, board->hcapacity * sizeof(*board->widths));
    }
    board->table[board->height] = row->items;
    board->widths[board->height] = row->size;
    ++board->height;
}

void add_square(trow *row, char c) {
    if (row->size == row->capacity) {
        row->capacity = RATE * row->capacity + 1;
        row->items = realloc(row->items, row->capacity * sizeof(*row->items));
    }
    row->items[row->size] = c;
    ++row->size;
}

bool is_lowercase(char c) {
    return (c >= 'a' && c <= 'z');
}

bool is_uppercase(char c) {
    return (c >= 'A' && c <= 'Z');
}

void init(tmove_stack *move_stack) {
    move_stack->capacity = 0;
    move_stack->top = 0;
    move_stack->items = NULL;
}

bool empty(tmove_stack *move_stack) {
    return move_stack->top == 0;
}

void pop(tmove_stack *move_stack, tmove *move) {
    *move = move_stack->items[move_stack->top - 1];
    --move_stack->top;
}

void reverse_move(tboard *board, tmove move) {
    int t = move.moved_box;
    update(board, board->box_positions[t], EMPTY_SQUARE);
    update(board, board->player_position, t + 'a');
    update(board, move.player_position, PLAYER_SQUARE);
    board->box_positions[t] = board->player_position;
    board->player_position = move.player_position;
}

/* If possible, push the box in this direction
   and put this move on the move stack */
void make_move(tboard *board, tmove_stack *move_stack, int box, int direction) {
    int x = board->box_positions[box].x;
    int y = board->box_positions[box].y;
    /* Position where box will be pushed. */
    tposition pos1;
    tposition pos2;
    /* Position where player must be in order to push the box */
    bool ok = true;
    if (direction == DOWN && is_avaliable(board, x, y + 1) &&
            is_avaliable(board, x, y - 1)) {
        pos1.x = x;
        pos1.y = y + 1;
        pos2.x = x;
        pos2.y = y - 1;
    } else if (direction == LEFT && is_avaliable(board, x - 1, y) &&
               is_avaliable(board, x + 1, y)) {
        pos1.x = x - 1;
        pos1.y = y;
        pos2.x = x + 1;
        pos2.y = y;
    } else if (direction == RIGHT && is_avaliable(board, x + 1, y) &&
               is_avaliable(board, x - 1, y)) {
        pos1.x = x + 1;
        pos1.y = y;
        pos2.x = x - 1;
        pos2.y = y;
    } else if (direction == UP && is_avaliable(board, x, y - 1) &&
               is_avaliable(board, x, y + 1)) {
        pos1.x = x;
        pos1.y = y - 1;
        pos2.x = x;
        pos2.y = y + 1;
    } else {
        ok = false;
    }
    if (ok) {
        tposition p = board->player_position;
        ok = is_path(board, p.x, p.y, pos2.x, pos2.y);
        reset(board);
        if (ok) {
            tmove move;
            move.moved_box = box;
            move.player_position = p;
            push(move_stack, move);
            update(board, p, EMPTY_SQUARE);
            board->player_position = board->box_positions[box];
            update(board, board->player_position, PLAYER_SQUARE);
            board->box_positions[box] = pos1;
            update(board, pos1, box + 'a');
        }
    }
}

/* Free memory allocated for the move stack */
void clear(tmove_stack *move_stack) {
    free(move_stack->items);
    free(move_stack);
}

void update(tboard *board, tposition p, char c) {
    int x = p.x;
    int y = p.y;
    if (is_target(board, p)) {
        if (is_lowercase(c)) {
            board->table[y][x] = c - 'a' + 'A';
        } else if (c == PLAYER_SQUARE) {
            board->table[y][x] = PLAYER_TARGET_SQUARE;
        } else if (c == EMPTY_SQUARE) {
            board->table[y][x] = EMPTY_TARGET_SQUARE;
        }
    } else {
        board->table[y][x] = c;
    }
}

/* Check if the player can stand on the position (x, y) */
bool is_avaliable(tboard *board, int x, int y) {
    if (y >= 0 && y < board->height &&
            x >= 0 && x < board->widths[y] &&
            (board->table[y][x] == EMPTY_SQUARE ||
             board->table[y][x] == EMPTY_TARGET_SQUARE ||
             board->table[y][x] == PLAYER_SQUARE ||
             board->table[y][x] == PLAYER_TARGET_SQUARE)) {
        return true;
    } else {
        return false;
    }
}

bool is_path(tboard *board, int x1, int y1, int x2, int y2) {
    if (x1 == x2 && y1 == y2) {
        return true;
    } else {
        char t = board->table[y1][x1];
        if (t == EMPTY_SQUARE) {
            board->table[y1][x1] = EMPTY_SQUARE_SEEN;
        } else if (t == EMPTY_TARGET_SQUARE) {
            board->table[y1][x1] = EMPTY_TARGET_SQUARE_SEEN;
        } else if (t == PLAYER_SQUARE) {
            board->table[y1][x1] = PLAYER_SQUARE_SEEN;
        } else if (t == PLAYER_TARGET_SQUARE) {
            board->table[y1][x1] = PLAYER_TARGET_SQUARE_SEEN;
        }
        bool ok = false;
        if (is_avaliable(board, x1, y1 + 1)) {
            ok = is_path(board, x1, y1 + 1, x2, y2);
        }
        if (!ok && is_avaliable(board, x1 - 1, y1)) {
            ok = is_path(board, x1 - 1, y1, x2, y2);
        }
        if (!ok && is_avaliable(board, x1 + 1, y1)) {
            ok = is_path(board, x1 + 1, y1, x2, y2);
        }
        if (!ok && is_avaliable(board, x1, y1 - 1)) {
            ok = is_path(board, x1, y1 - 1, x2, y2);
        }
        return ok;
    }
}

/* Reset the positions checked by is_path function. */
void reset(tboard *board) {
    for (int y = 0; y < board->height; ++y) {
        for (int x = 0; x < board->widths[y]; ++x) {
            char t = board->table[y][x];
            if (t == EMPTY_SQUARE_SEEN) {
                board->table[y][x] = EMPTY_SQUARE;
            } else if (t == EMPTY_TARGET_SQUARE_SEEN) {
                board->table[y][x] = EMPTY_TARGET_SQUARE;
            } else if (t == PLAYER_SQUARE_SEEN) {
                board->table[y][x] = PLAYER_SQUARE;
            } else if (t == PLAYER_TARGET_SQUARE_SEEN) {
                board->table[y][x] = PLAYER_TARGET_SQUARE;
            }
        }
    }
}

void push(tmove_stack *move_stack, tmove move) {
    if (move_stack->top == move_stack->capacity) {
        move_stack->capacity = RATE * move_stack->capacity + 1;
        move_stack->items = realloc(move_stack->items, move_stack->capacity * sizeof(*move_stack->items));
    }
    move_stack->items[move_stack->top] = move;
    ++move_stack->top;
}

bool is_target(tboard *board, tposition p) {
    int x = p.x;
    int y = p.y;
    if (board->table[y][x] == EMPTY_TARGET_SQUARE ||
            board->table[y][x] == PLAYER_TARGET_SQUARE ||
            is_uppercase(board->table[y][x])) {
        return true;
    } else {
        return false;
    }
}

