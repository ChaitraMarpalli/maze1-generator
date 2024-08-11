#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define CELL_SIZE 20
#define ROWS (HEIGHT / CELL_SIZE)
#define COLS (WIDTH / CELL_SIZE)

typedef struct {
    int x, y;
} Cell;

Cell player, goal;
int maze[ROWS][COLS];
Cell stack[ROWS * COLS];
int stack_size = 0;

void push(Cell c) {
    stack[stack_size++] = c;
}

Cell pop() {
    return stack[--stack_size];
}

int is_empty() {
    return stack_size == 0;
}

void draw_maze(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (maze[i][j] & 1) SDL_RenderDrawLine(renderer, j * CELL_SIZE, i * CELL_SIZE, (j + 1) * CELL_SIZE, i * CELL_SIZE); // Top
            if (maze[i][j] & 2) SDL_RenderDrawLine(renderer, (j + 1) * CELL_SIZE, i * CELL_SIZE, (j + 1) * CELL_SIZE, (i + 1) * CELL_SIZE); // Right
            if (maze[i][j] & 4) SDL_RenderDrawLine(renderer, j * CELL_SIZE, (i + 1) * CELL_SIZE, (j + 1) * CELL_SIZE, (i + 1) * CELL_SIZE); // Bottom
            if (maze[i][j] & 8) SDL_RenderDrawLine(renderer, j * CELL_SIZE, i * CELL_SIZE, j * CELL_SIZE, (i + 1) * CELL_SIZE); // Left
        }
    }

    // Draw player
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect playerRect = { player.x * CELL_SIZE + 1, player.y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2 };
    SDL_RenderFillRect(renderer, &playerRect);

    // Draw goal
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect goalRect = { goal.x * CELL_SIZE + 1, goal.y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2 };
    SDL_RenderFillRect(renderer, &goalRect);

    SDL_RenderPresent(renderer);
}

void generate_maze() {
    Cell start = {0, 0};
    push(start);
    maze[start.y][start.x] = 15; // Initialize with all walls

    while (!is_empty()) {
        Cell current = pop();
        int directions[] = {1, 2, 4, 8};
        for (int i = 0; i < 4; i++) {
            int j = rand() % 4;
            int temp = directions[i];
            directions[i] = directions[j];
            directions[j] = temp;
        }

        for (int i = 0; i < 4; i++) {
            int nx = current.x, ny = current.y;
            int direction = directions[i];
            switch (direction) {
                case 1: ny--; break; // Up
                case 2: nx++; break; // Right
                case 4: ny++; break; // Down
                case 8: nx--; break; // Left
            }

            if (nx >= 0 && ny >= 0 && nx < COLS && ny < ROWS && maze[ny][nx] == 0) {
                maze[current.y][current.x] &= ~direction;
                maze[ny][nx] = 15;
                switch (direction) {
                    case 1: maze[ny][nx] &= ~4; break; // Remove bottom wall of the new cell
                    case 2: maze[ny][nx] &= ~8; break; // Remove left wall of the new cell
                    case 4: maze[ny][nx] &= ~1; break; // Remove top wall of the new cell
                    case 8: maze[ny][nx] &= ~2; break; // Remove right wall of the new cell
                }
                push(current);
                Cell next = {nx, ny};
                push(next);
                break;
            }
        }
    }
}

void initialize_game() {
    player.x = 0;
    player.y = 0;
    goal.x = COLS - 1;
    goal.y = ROWS - 1;
    maze[player.y][player.x] = 0; // Ensure the player can move
    maze[goal.y][goal.x] = 0;     // Ensure the goal is reachable
}

void handle_input(SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        int new_x = player.x, new_y = player.y;
        switch (event.key.keysym.sym) {
            case SDLK_UP:    if (!(maze[player.y][player.x] & 1)) new_y--; break; // Move up if no top wall
            case SDLK_RIGHT: if (!(maze[player.y][player.x] & 2)) new_x++; break; // Move right if no right wall
            case SDLK_DOWN:  if (!(maze[player.y][player.x] & 4)) new_y++; break; // Move down if no bottom wall
            case SDLK_LEFT:  if (!(maze[player.y][player.x] & 8)) new_x--; break; // Move left if no left wall
        }
        if (new_x >= 0 && new_y >= 0 && new_x < COLS && new_y < ROWS) {
            player.x = new_x;
            player.y = new_y;
        }
    }
}

int check_win() {
    return player.x == goal.x && player.y == goal.y;
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Maze Generator and Solver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    srand(time(NULL));
    generate_maze();
    initialize_game();

    int quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else {
                handle_input(event);
            }
        }

        draw_maze(renderer);

        if (check_win()) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Congratulations", "You reached the goal!", window);
            quit = 1;
        }

        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}