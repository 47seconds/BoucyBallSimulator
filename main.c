#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdint.h>
// #include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_ORANGE 0xFFA500FF
#define RADIUS 50
#define INIT_XVEL 0
#define INIT_YVEL 0
#define GRAVITY 0.2
#define INVERSE_FRICTION_COEFF 0.996
#define X_DAMP_COEFF 0.95
#define Y_DAMP_COEFF 0.8
#define MIN_YVEL 1
#define MIN_XVEL 1
#define PATH_TRACE_LENGTH 30
#define getFPS(FPS) 1000/FPS
#define SIMULATION_FPS 144

int WIDTH = 900, HEIGHT = 600;

typedef struct Point {
  double x;
  double y;
  struct Point* next;
} Point ;

typedef struct Circle {
  double x;
  double y;
  int radius;
  double xvel;
  double yvel;
} Circle;

typedef struct Path {
  Point* st;
  Point* top;
  int n_points;
} Path;

void enterFullScreen(SDL_Window* window) {
  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_GetWindowSize(window, &WIDTH, &HEIGHT);
}

void setRendererDrawColor(SDL_Renderer* renderer, Uint32 color) {
  Uint8 r = (color >> 24) & 0xFF;
  Uint8 g = (color >> 16) & 0xFF;
  Uint8 b = (color >> 8) & 0xFF;
  Uint8 a = color & 0xFF;

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

Point* createPoint(double x, double y) {
  Point* pt = (Point*)malloc(sizeof(Point));
  pt->x = x;
  pt->y = y;
  pt->next = NULL;
  return pt;
}

Circle* createBall(Point* start, int radius, double xvel, double yvel) {
  Circle* circle = (Circle*)malloc(sizeof(Circle));
  circle->x = start->x;
  circle->y = start->y;
  circle->radius = radius;
  circle->xvel = xvel;
  circle->yvel = yvel;
  return circle;
}

Path* createPath(int pathLen, Point* start) {
  Path* path = (Path*)malloc(sizeof(Path));
  path->st = start;
  path->top = start;
  path->n_points = 1;
  return path;
}

void nextPointIntoPath(Path* path, Point* pt) {
  path->top->next = pt;
  path->top = path->top->next;
  path->n_points++;

  if (path->n_points > PATH_TRACE_LENGTH) {
    Point* temp = path->st;
    path->st = path->st->next;
    free(temp);
    path->n_points--;
  }

  // printf("pt->y: %f\tpath->top->y: %f\n", pt->y, path->top->y);
}

void printPath(Path* path) {
  Point* tr = path->st;
  int cnt = path->n_points;
  while(tr) {
    // printf("%d\t%f\n", cnt, tr->y);
    tr = tr->next;
    cnt--;
  }
}

void drawPath(SDL_Renderer* renderer, Path* path) {
  Point* tr = path->st;
  while (tr->next) {
    SDL_RenderDrawLine(renderer, tr->x, tr->y, tr->next->x, tr->next->y);
    tr = tr->next;
  }
}

void deletePath(Path* path) {
  while (path->st) {
    Point* temp = path->st;
    // printf("%d\t%f\n", path->n_points--, temp->y);
    path->st = path->st->next;
    free(temp);
  }
  free(path);
}

void drawCircle(SDL_Renderer* renderer, Circle* circle, Uint32 color) {
  setRendererDrawColor(renderer, color);

  // Using Modified Bresenham's Circle Algorithm
  int x = 0, y = circle->radius;
  int dp = 3 - 2 * circle->radius;  // decision perimeter

  while (x <= y) {
      // Draw horizontal lines to fill the circle
      SDL_RenderDrawLine(renderer, circle->x - x, circle->y - y, circle->x + x, circle->y - y);
      SDL_RenderDrawLine(renderer, circle->x - x, circle->y + y, circle->x + x, circle->y + y);
      SDL_RenderDrawLine(renderer, circle->x - y, circle->y - x, circle->x + y, circle->y - x);
      SDL_RenderDrawLine(renderer, circle->x - y, circle->y + x, circle->x + y, circle->y + x);

      if (dp < 0) {
          dp = dp + 4 * x + 6;
      } else {
          dp = dp + 4 * (x - y) + 10;
          y--;
      }
      x++;
  }
}

void applyGravity(Circle* ball) {
  ball->y += ball->yvel;
  ball->x += ball->xvel;
  if (ball->y < HEIGHT - ball->radius) ball->yvel += GRAVITY;
}

// Damping logic flawed maybe, ball doesn't seem to stop
void reflectionFrictionAndDamping(Circle* ball) {
  if (ball->y >= HEIGHT - ball->radius || ball->y < ball->radius) {
    if ((ball-> y >= HEIGHT - ball->radius || ball->y <= ball->radius) && fabs(ball->yvel) <= MIN_YVEL) ball->yvel = 0;
    else ball->yvel = (-1) * (ball->yvel * Y_DAMP_COEFF);

    if (ball-> y >= HEIGHT - ball->radius) ball->y = HEIGHT - ball->radius;
    else ball->y = ball->radius;

    ball->xvel = (ball->xvel * INVERSE_FRICTION_COEFF);
  }

  if (ball->x >= WIDTH - ball->radius || ball->x < ball->radius) {
    if (((ball->x >= WIDTH - ball->radius || ball->x <= ball->radius) && fabs(ball->xvel) <= MIN_XVEL)) ball->xvel = 0;
    else ball->xvel = (-1) * (ball->xvel * X_DAMP_COEFF);
    if (ball-> x >= WIDTH - ball->radius) ball->x = WIDTH - ball->radius;
    else ball->x = ball->radius;
  }

  // printf("vvel: %f, y: %f\n", ball->yvel, ball->y);
}

void calculateTrajectory (Circle* ball, Path* path, int* justReleasedMouse) {
  float xvel = 0, yvel = 0;
  Point* tr = path->st;
  while (tr->next) {
    xvel += tr->next->x - tr->x;
    yvel += tr->next->y - tr->y;
    tr = tr->next;
  } 
  ball->xvel = xvel/path->n_points;
  ball->yvel = yvel/path->n_points;
  *justReleasedMouse = 0;
}

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Bouncy Ball Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  enterFullScreen(window);

  setRendererDrawColor(renderer, COLOR_BLACK);
  SDL_RenderClear(renderer);

  Point* start = createPoint((double) WIDTH/2, (double) HEIGHT/2);

  Circle* ball = createBall(start, RADIUS, INIT_XVEL, INIT_YVEL);
  drawCircle(renderer, ball, COLOR_WHITE);
  SDL_RenderPresent(renderer);

  Path* path = createPath(PATH_TRACE_LENGTH, start);

  SDL_Event event;
  int simulation_running = 1;
  int mousePressed = 0;
  int justReleasedMouse = 0;
  while (simulation_running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          simulation_running = 0;
          break;
        
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              simulation_running = 0;
              break;
          }

        case SDL_MOUSEBUTTONDOWN:
          mousePressed = 1;
          ball->x = event.button.x;
          ball->y = event.button.y;
          break;

        case SDL_MOUSEBUTTONUP:
          mousePressed = 0;
          justReleasedMouse = 1;
          break;

        case SDL_MOUSEMOTION:
          if (mousePressed) {
            ball->x = event.button.x;
            ball->y = event.button.y;
          }
          break;
      }
    }

    setRendererDrawColor(renderer, COLOR_BLACK);
    SDL_RenderClear(renderer);

    if (!mousePressed) applyGravity(ball);
    if (justReleasedMouse) calculateTrajectory(ball, path, &justReleasedMouse);
    reflectionFrictionAndDamping(ball);

    setRendererDrawColor(renderer, COLOR_ORANGE);
    Point* nextPoint = createPoint(ball->x, ball->y);
    nextPointIntoPath(path, nextPoint);
    drawPath(renderer, path);

    drawCircle(renderer, ball, COLOR_WHITE);
    SDL_RenderPresent(renderer);

    SDL_Delay(getFPS(SIMULATION_FPS));
  }

  // printf("\n\n");
  // if (start) free(start); // THIS MF MADE ME DEBUG DOUBLE FREE()~ MALLOC ERROR FOR 4 FKING HOURS!!
  // printPath(path);
  // printf("\n\n");
  deletePath(path);
  free(ball);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
