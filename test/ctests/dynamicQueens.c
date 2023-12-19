#include "stdlib/stdio.h"

int N = 0;
int numberOfSolutions = 0;

void printSolution(int **board) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            char cell = board[i][j] == 1 ? '@' : '.';
            putchar(cell);
            putchar(' ');
        }
        putchar('\n');
    }
}

int isSafe(int **board, int row, int col) {
    int i, j;

    for (i = 0; i < col; i++)
        if (board[row][i])
            return 0;

    for (i = row, j = col; i >= 0 && j >= 0; i--, j--)
        if (board[i][j])
            return 0;

    for (i = row, j = col; j >= 0 && i < N; i++, j--)
        if (board[i][j])
            return 0;

    return 1;
}

int solveNQUtil(int **board, int col, int depth) {
    if (col >= N)
        return 1;

    for (int i = 0; i < N; i++) {
        if (isSafe(board, i, col)) {
            board[i][col] = 1;
            if (solveNQUtil(board, col + 1, depth + 1)) {
                if (depth == N - 1) {
                    numberOfSolutions++;
                    printf("Found solution #%d:\n", numberOfSolutions);
                    printSolution(board);
                    putchar('\n');
                }
            }
            board[i][col] = 0;
        }
    }
    if (depth == 0 && numberOfSolutions == 0) {
        puts("Solution does not exist!");
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <num_queens>\n", argv[0]);
        return 0;
    }

    N = atoi(argv[1]);

    int **board = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        board[i] = (int *)malloc(N * sizeof(int));
    }

    solveNQUtil(board, 0, 0);

    printf("Total amount of solutions: %d\n", numberOfSolutions);

    for (int i = 0; i < N; i++) {
        free(board[i]);
    }
    free(board);

    return 0;
}
