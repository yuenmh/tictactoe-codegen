#include <limits.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int8_t Sym;

#define SYM_INVALID -1
#define SYM_EMPTY 0
#define SYM_SELF 1
#define SYM_OPP 2

/// Gets the inverse of the symbol. Returns the same symbol if it is not a
/// player symbol.
Sym sym_invert(Sym sym) {
    switch (sym) {
    case SYM_SELF:
        return SYM_OPP;
    case SYM_OPP:
        return SYM_SELF;
    default:
        return sym;
    }
}

/// Determines if the symbol is a player symbol.
bool sym_is_player(Sym sym) { return sym == SYM_SELF || sym == SYM_OPP; }

typedef struct {
    int r;
    int c;
} Move;

#define MOVE_INVALID ((Move){.r = -1, .c = -1})

Move move_create(int r, int c) { return (Move){.r = r, .c = c}; }

void move_print(Move *move) { printf("(%d, %d)", move->r, move->c); }

bool move_is_valid(Move *move) { return move->r >= 0 && move->c >= 0; }

bool move_equals(Move *move, Move *other) {
    return move->r == other->r && move->c == other->c;
}

#define BOARD_SIZE 3
#define BOARD_SIZE_SQ (BOARD_SIZE * BOARD_SIZE)

typedef struct {
    Sym cells[BOARD_SIZE_SQ];
} Board;

Board board_create(void) {
    Board board;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        board.cells[i] = SYM_EMPTY;
    }
    return board;
}

Board board_clone(Board *board) {
    Board new_board;
    memcpy(&new_board, board, sizeof(Board));
    return new_board;
}

Sym board_get_sym(Board *board, int r, int c) {
    int index = r * BOARD_SIZE + c;
    // Ensure that the index is valid
    if (index < 0 || index >= BOARD_SIZE_SQ) {
        return SYM_INVALID;
    }
    return board->cells[index];
}

void board_set_sym_mut(Board *board, int r, int c, Sym sym) {
    int index = r * BOARD_SIZE + c;
    // Ensure that the index is valid
    if (index < 0 || index >= BOARD_SIZE_SQ) {
        return;
    }
    board->cells[index] = sym;
}

Board board_set_sym(Board *board, int r, int c, Sym sym) {
    Board new_board = board_clone(board);
    board_set_sym_mut(&new_board, r, c, sym);
    return new_board;
}

void board_print(Board *board) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Sym sym = board_get_sym(board, r, c);
            switch (sym) {
            case SYM_EMPTY:
                printf(" ");
                break;
            case SYM_SELF:
                printf("X");
                break;
            case SYM_OPP:
                printf("O");
                break;
            }
            if (c < BOARD_SIZE - 1) {
                printf("|");
            }
        }
        printf("\n");
        if (r < BOARD_SIZE - 1) {
            for (int i = 0; i < BOARD_SIZE; i++) {
                printf("-");
                if (i < BOARD_SIZE - 1) {
                    printf("+");
                }
            }
            printf("\n");
        }
    }
}

/// Gets the winner of the board. Returns `SYM_EMPTY` if there is no winner.
/// This means that if it is a tie or the game is not over yet, `SYM_EMPTY` is
/// returned.
Sym board_get_winner(Board *board) {
    // Check rows
    for (int r = 0; r < BOARD_SIZE; r++) {
        Sym first = board_get_sym(board, r, 0);
        if (first == SYM_EMPTY) {
            continue;
        }
        bool win = true;
        for (int c = 1; c < BOARD_SIZE; c++) {
            Sym sym = board_get_sym(board, r, c);
            if (sym != first) {
                win = false;
                break;
            }
        }
        if (win) {
            return first;
        }
    }
    // Check columns
    for (int c = 0; c < BOARD_SIZE; c++) {
        Sym first = board_get_sym(board, 0, c);
        if (first == SYM_EMPTY) {
            continue;
        }
        bool win = true;
        for (int r = 1; r < BOARD_SIZE; r++) {
            Sym sym = board_get_sym(board, r, c);
            if (sym != first) {
                win = false;
                break;
            }
        }
        if (win) {
            return first;
        }
    }
    // Check tl-br diagonal
    Sym first = board_get_sym(board, 0, 0);
    if (first != SYM_EMPTY) {
        bool win = true;
        for (int i = 1; i < BOARD_SIZE; i++) {
            Sym sym = board_get_sym(board, i, i);
            if (sym != first) {
                win = false;
                break;
            }
        }
        if (win) {
            return first;
        }
    }
    // Check tr-bl diagonal
    first = board_get_sym(board, 0, BOARD_SIZE - 1);
    if (first != SYM_EMPTY) {
        bool win = true;
        for (int i = 1; i < BOARD_SIZE; i++) {
            Sym sym = board_get_sym(board, i, BOARD_SIZE - 1 - i);
            if (sym != first) {
                win = false;
                break;
            }
        }
        if (win) {
            return first;
        }
    }
    return SYM_EMPTY;
}

bool board_move_is_possible(Board *board, int r, int c) {
    return board_get_sym(board, r, c) == SYM_EMPTY;
}

bool board_is_full(Board *board) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (board_get_sym(board, r, c) == SYM_EMPTY) {
                return false;
            }
        }
    }
    return true;
}

/// Gets the winner of the board. Returns `SYM_INVALID` if there is a tie, and
/// `SYM_EMPTY` if the game is not over yet.
Sym board_get_winner_or_tie(Board *board) {
    Sym winner = board_get_winner(board);
    if (winner == SYM_EMPTY && board_is_full(board)) {
        // Tie condition
        return SYM_INVALID;
    } else {
        return winner;
    }
}

typedef struct {
    Move move;
    int score;
} ScoredMove;

/// Returns the best move and score for the given symbol.
/// @param board The board to search
/// @param sym The current symbol
/// @param maximizing_sym The symbol that is trying to maximize the score
ScoredMove get_best_move_and_score(Board *board, Sym sym, Sym maximizing_sym) {
    // Base case
    Sym winner = board_get_winner(board);
    if (winner == maximizing_sym) {
        return (ScoredMove){MOVE_INVALID, 1};
    } else if (winner == sym_invert(maximizing_sym)) {
        return (ScoredMove){MOVE_INVALID, -1};
    } else if (board_is_full(board)) {
        return (ScoredMove){MOVE_INVALID, 0};
    }
    // Iterate through all the possible moves, there should be at least one
    // possible move because of the base case
    // TODO Find a way to de-duplicate this code. Maybe a function pointer?
    if (sym == maximizing_sym) {
        // Maximizing branch
        int value = INT_MIN;
        Move move = MOVE_INVALID;
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (!board_move_is_possible(board, r, c)) {
                    continue;
                }
                Board new_board = board_set_sym(board, r, c, sym);
                // Looping possible moves, now we know the move is possible
                int new_value = get_best_move_and_score(
                                    &new_board, sym_invert(sym), maximizing_sym)
                                    .score;
                if (new_value > value) {
                    value = new_value;
                    move = move_create(r, c);
                }
            }
        }
        return (ScoredMove){
            .move = move,
            .score = value,
        };
    } else {
        // Minimizing branch
        int value = INT_MAX;
        Move move = MOVE_INVALID;
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (!board_move_is_possible(board, r, c)) {
                    continue;
                }
                Board new_board = board_set_sym(board, r, c, sym);
                // Looping possible moves, now we know the move is possible
                int new_value = get_best_move_and_score(
                                    &new_board, sym_invert(sym), maximizing_sym)
                                    .score;
                if (new_value < value) {
                    value = new_value;
                    move = move_create(r, c);
                }
            }
        }
        return (ScoredMove){
            .move = move,
            .score = value,
        };
    }
}

Move get_best_move(Board *board, Sym sym, Sym maximizing_sym) {
    return get_best_move_and_score(board, sym, maximizing_sym).move;
}

void run_game(void) {
    Board board = board_create();
    while (true) {
        int r, c;
        // Loop until valid input
        while (true) {
            printf("Enter row,column: ");
            scanf("%d,%d", &r, &c);
            if (!board_move_is_possible(&board, r, c)) {
                printf("Invalid move\n");
                continue;
            }
            break;
        }
        // Set the player's move
        board_set_sym_mut(&board, r, c, SYM_OPP);
        board_print(&board);
        {
            // Check the winner and break if the game is over
            Sym winner = board_get_winner_or_tie(&board);
            if (winner == SYM_INVALID) {
                printf("Tie!\n");
                break;
            } else if (winner == SYM_SELF) {
                printf("You lose!\n");
                break;
            } else if (winner == SYM_OPP) {
                printf("You win!\n");
                break;
            }
        }
        // Computer's turn
        Move my_move = get_best_move(&board, SYM_SELF, SYM_SELF);
        printf("Computer's move: %d,%d\n", my_move.r, my_move.c);
        board_set_sym_mut(&board, my_move.r, my_move.c, SYM_SELF);
        board_print(&board);
        {
            // Check the winner and break if the game is over
            Sym winner = board_get_winner_or_tie(&board);
            if (winner == SYM_INVALID) {
                printf("Tie!\n");
                break;
            } else if (winner == SYM_SELF) {
                printf("You lose!\n");
                break;
            } else if (winner == SYM_OPP) {
                printf("You win!\n");
                break;
            }
        }
    }
}

void generate_board_print(Board *board) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("printf(\"");
        for (int c = 0; c < BOARD_SIZE; c++) {
            Sym sym = board_get_sym(board, r, c);
            if (sym == SYM_SELF) {
                printf(" X ");
            } else if (sym == SYM_OPP) {
                printf(" O ");
            } else {
                printf(" - ");
            }
        }
        printf("\\n\");\n");
    }
}

void generate_turn(Board *board, Sym auto_player) {
    Sym user_player = sym_invert(auto_player);
    // Outer loop for the user input because some of the moves don't lead to
    // the next state
    printf("while (1) {\n");
    printf("input = get_input();\n");
    bool is_first_iter = true;
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            // Turn it into else if, if it's not the first iteration
            if (!is_first_iter) {
                printf("else ");
            }
            printf("if (input.r == %d && input.c == %d) {\n", r, c);
            is_first_iter = false;
            if (!board_move_is_possible(board, r, c)) {
                printf("    printf(\"Move is not possible\\n\");\n");
                printf("    continue;\n");
            } else {
                // Print the user's board
                Board user_move_board = board_set_sym(board, r, c, user_player);
                generate_board_print(&user_move_board);
                // There is only one best move per possible user move
                Move my_move =
                    get_best_move(&user_move_board, auto_player, auto_player);
                printf("    printf(\"Computer's move: %d,%d\\n\");\n",
                       my_move.r, my_move.c);
                // Print the computer's board
                Board my_move_board = board_set_sym(&user_move_board, my_move.r,
                                                    my_move.c, auto_player);
                generate_board_print(&my_move_board);
                // Check if the game is over
                Sym winner = board_get_winner_or_tie(&my_move_board);
                if (winner == SYM_INVALID) {
                    printf("    printf(\"Tie!\\n\");\n");
                } else if (winner == SYM_SELF) {
                    printf("    printf(\"You lose!\\n\");\n");
                } else if (winner == SYM_OPP) {
                    printf("    printf(\"You win!\\n\");\n");
                } else {
                    // Continue recursing
                    generate_turn(&my_move_board, auto_player);
                }
                printf("    break;\n");
            }
            printf("}\n");
        }
    }
    // The default case
    printf("else {\n");
    printf("    printf(\"Move is not possible\\n\");\n");
    printf("    continue;\n");
    printf("}\n");
    // closing brace for the while loop
    printf("}\n");
}

void generate_code(void) {
    char *code = "#include <stdio.h>\n"
                 "\n"
                 "typedef struct { int r, c; } Move;\n"
                 "\n"
                 "Move get_input(void) {\n"
                 "    int r, c;\n"
                 "    while (1) {\n"
                 "        r = -1;\n"
                 "        c = -1;\n"
                 "        printf(\"Enter row,column: \");\n"
                 // FIXME probably have to manually parse it
                 "        scanf(\"%d,%d\", &r, &c);\n"
                 "        if (r < 0 || c < 0 || r > 2 || c > 2) {\n"
                 "            printf(\"Invalid move\\n\");\n"
                 "            continue;\n"
                 "        }\n"
                 "        break;\n"
                 "    }\n"
                 "    return (Move){.r = r, .c = c};\n"
                 "}\n"
                 "\n"
                 "int main(void) {\n"
                 "    printf(\"You are O, the computer is X.\\n\");\n"
                 "    Move input = (Move){.r = -1, .c = -1};\n";
    printf("%s", code);
    Board board = board_create();
    generate_turn(&board, SYM_SELF);
    printf("}\n");
}

int main(void) {
    // printf("You are O, the computer is X.\n");
    // run_game();
    generate_code();
    return 0;
}
