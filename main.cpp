#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <set>
#include <string>

// Window and grid settings
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SPACING = 20;

using Grid = std::vector<std::vector<bool>>;

int countLiveNeighbors(const Grid& grid, int row, int col) {
    int liveCount = 0;
    int rows = grid.size();
    int cols = grid[0].size();

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dy == 0 && dx == 0) continue;
            int y = row + dy;
            int x = col + dx;
            if (y >= 0 && y < rows && x >= 0 && x < cols) {
                if (grid[y][x]) ++liveCount;
            }
        }
    }
    return liveCount;
}

void updateGrid(Grid& grid) {
    int rows = grid.size();
    int cols = grid[0].size();
    Grid next = grid;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int liveNeighbors = countLiveNeighbors(grid, row, col);
            if (grid[row][col]) {
                next[row][col] = (liveNeighbors == 2 || liveNeighbors == 3);
            } else {
                next[row][col] = (liveNeighbors == 3);
            }
        }
    }

    grid = next;
}

std::string gridToString(const Grid& grid) {
    std::string s;
    for (const auto& row : grid) {
        for (bool cell : row) {
            s += cell ? '1' : '0';
        }
    }
    return s;
}

bool isEmpty(const Grid& grid) {
    for (const auto& row : grid) {
        for (bool cell : row) {
            if (cell) return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Conway's Game of Life",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int cols = WINDOW_WIDTH / GRID_SPACING;
    int rows = WINDOW_HEIGHT / GRID_SPACING;
    Grid filled(rows, std::vector<bool>(cols, false));

    bool quit = false;
    bool running = false;
    bool mouseDown = false;
    bool stopped = false;
    std::set<std::string> previousStates;
    SDL_Event e;
    Uint32 lastUpdate = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && !running) {
                running = true;
                std::cout << "Simulation started.\n";
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && !running) {
                mouseDown = true;
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                mouseDown = false;
            } else if (e.type == SDL_MOUSEMOTION && mouseDown && !running) {
                int x = e.motion.x;
                int y = e.motion.y;
                int col = x / GRID_SPACING;
                int row = y / GRID_SPACING;
                if (row >= 0 && row < rows && col >= 0 && col < cols) {
                    filled[row][col] = true;
                }
            }
        }

        if (running && !stopped && SDL_GetTicks() - lastUpdate > 200) {
            std::string current = gridToString(filled);

            // Check if pattern is repeating or grid is empty
            if (previousStates.count(current)) {
                std::cout << "Pattern entered a loop. Simulation stopped.\n";
                stopped = true;
            } else if (isEmpty(filled)) {
                std::cout << "No live cells remain. Simulation stopped.\n";
                stopped = true;
            } else {
                previousStates.insert(current);
                updateGrid(filled);
            }

            lastUpdate = SDL_GetTicks();
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw filled cells
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (filled[row][col]) {
                    SDL_Rect cell = {
                        col * GRID_SPACING,
                        row * GRID_SPACING,
                        GRID_SPACING,
                        GRID_SPACING
                    };
                    SDL_RenderFillRect(renderer, &cell);
                }
            }
        }

        // Draw grid lines
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        for (int x = 0; x < WINDOW_WIDTH; x += GRID_SPACING)
            SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
        for (int y = 0; y < WINDOW_HEIGHT; y += GRID_SPACING)
            SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
