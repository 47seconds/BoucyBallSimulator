#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_ORANGE 0xFFA500FF
#define COLOR_RED 0xFF000000
#define COLOR_GREEN 0x00FF00FF
#define COLOR_BLUE 0x0000FFFF
#define COLOR_PURPLE 0x800080FF
#define COLOR_LIME 0x00FF00FF
#define COLOR_FUCHSLA 0xFF00FFFF
#define COLOR_MAROON 0x800000FF
#define COLOR_NAVY 0x000080FF
#define COLOR_YELLOW 0xFFFF00FF
#define COLOR_AQUA 0x00FFFFFF
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
#define TRAJECTORY_AVG_SIZE 2
#define TRAJECTORY_CALCULATION_WEIGHT 200
#define MOUSE_SENSITIVITY 0.4
#define N_BALLS 5
#define getFPS(FPS) 1000/FPS
#define SIMULATION_FPS 144

int WIDTH = 900, HEIGHT = 600;

Uint32 COLORS[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_ORANGE, COLOR_PURPLE, COLOR_LIME, COLOR_FUCHSLA, COLOR_MAROON, COLOR_NAVY, COLOR_YELLOW, COLOR_AQUA, COLOR_WHITE};

typedef struct Point {
  double x;
  double y;
  struct Point* next;
  struct Point* prev;
} Point ;

typedef struct Path {
  Point* st;
  Point* top;
  int n_points;
} Path;

typedef struct Circle {
  Point* coords;
  int radius;
  double xvel;
  double yvel;
  Uint32 color;
  Path* path;
} Circle;

// BALL->COORDS WILL BE HEAD AND PATH WILL MAKE POINTS TRACING THESE HEAD POINTS TO MAKE PATH LINKED LIST 

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
  pt->prev = NULL;
  return pt;
}

Path* createPath(int pathLen, Point* start) {
  Point* pt = createPoint(start->x, start->y);
  Path* path = (Path*)malloc(sizeof(Path));
  path->st = pt;
  path->top = pt;
  path->n_points = 1;
  return path;
}

// path linked list contains points from ball->coords itself, so no need to free them twice
Circle* createBall(Point* start, int radius, double xvel, double yvel, Uint32 color) {
  Circle* circle = (Circle*)malloc(sizeof(Circle));
  circle->coords = start;
  circle->radius = radius;
  circle->xvel = xvel;
  circle->yvel = yvel;
  circle->color = color;
  circle->path = createPath(PATH_TRACE_LENGTH, start);
  return circle;
}

Circle** createBalls(int n) {
  Circle** balls = (Circle**)malloc(n * sizeof(Circle*));
  for (int i = 0; i < n; i++) {
    Point* st = createPoint((float)(i + 1) * ((float)WIDTH / (n + 1)), (float)WIDTH / 2);
    balls[i] = createBall(st, RADIUS, 0, 0, COLORS[rand() % (sizeof(COLORS) / sizeof(Uint32))]);
  }
  return balls;
}

void nextPointIntoPath(Circle* ball) {
  Point* pt = createPoint(ball->coords->x, ball->coords->y);

  ball->path->top->next = pt;
  pt->prev = ball->path->top;
  ball->path->top = pt;
  ball->path->n_points++;

  if (ball->path->n_points > PATH_TRACE_LENGTH) {
    Point* temp = ball->path->st;
    ball->path->st = ball->path->st->next;
    ball->path->st->prev = NULL;
    free(temp);
    ball->path->n_points--;
  }

  // printf("pt->y: %f\tpath->top->y: %f\n", pt->y, path->top->y);
}

void nextPointsIntoPaths(Circle** balls, int n) {
  for (int i = 0; i < n; i++) nextPointIntoPath(balls[i]);
}

void printPath(Path* path) {
  Point* tr = path->st;
  int cnt = path->n_points;
  while(tr) {
    printf("%d\t%f\n", cnt, tr->y);
    tr = tr->next;
    cnt--;
  }
}

void drawPathForBall(SDL_Renderer* renderer, Circle* ball, int i) {
  setRendererDrawColor(renderer, ball->color);
  Point* tr = ball->path->st;
  while (tr->next) {
    SDL_RenderDrawLine(renderer, tr->x, tr->y, tr->next->x, tr->next->y);
    tr = tr->next;
  }
}

void drawPaths(SDL_Renderer* renderer, Circle** balls, int n) {
  for (int i = 0; i < n; i++) drawPathForBall(renderer, balls[i], i);
}

void deletePath(Circle* ball) {
  Point* tr = ball->path->st;
  while (tr) {
    Point* temp = tr;
    // printf("%d\t%f\n", path->n_points--, temp->y);
    tr = tr->next;
    free(temp);
  }
  free(ball->path);
}

void drawCircle(SDL_Renderer* renderer, Circle* circle) {
  setRendererDrawColor(renderer, circle->color);

  // Using Modified Bresenham's Circle Algorithm
  int x = 0, y = circle->radius;
  int dp = 3 - 2 * circle->radius;  // decision perimeter

  while (x <= y) {
      // Draw horizontal lines to fill the circle
      SDL_RenderDrawLine(renderer, circle->coords->x - x, circle->coords->y - y, circle->coords->x + x, circle->coords->y - y);
      SDL_RenderDrawLine(renderer, circle->coords->x - x, circle->coords->y + y, circle->coords->x + x, circle->coords->y + y);
      SDL_RenderDrawLine(renderer, circle->coords->x - y, circle->coords->y - x, circle->coords->x + y, circle->coords->y - x);
      SDL_RenderDrawLine(renderer, circle->coords->x - y, circle->coords->y + x, circle->coords->x + y, circle->coords->y + x);

      if (dp < 0) {
          dp = dp + 4 * x + 6;
      } else {
          dp = dp + 4 * (x - y) + 10;
          y--;
      }
      x++;
  }
}

void drawBalls(SDL_Renderer* renderer, Circle** balls, int n) {
  for (int i = 0; i < n; i++) drawCircle(renderer, balls[i]);
}

void deleteBall(Circle* ball) {
  deletePath(ball);
  // free(ball->coords); // Since ball->path->top is ball->coords themselves, double free()
  free(ball);
}

void deleteBalls(Circle** balls, int n) {
  for (int i = 0; i < n; i++) deleteBall(balls[i]);
  free(balls);
}

void applyGravityToBall(Circle* ball) {
  ball->coords->y += ball->yvel;
  ball->coords->x += ball->xvel;
  if (ball->coords->y < HEIGHT - ball->radius) ball->yvel += GRAVITY;
}

void applyGravity(Circle** balls, int n) {
  for (int i = 0; i < n; i++) applyGravityToBall(balls[i]);
}

void reflectionFrictionAndDampingToBall(Circle* ball) {
  if (ball->coords->y >= HEIGHT - ball->radius || ball->coords->y < ball->radius) {
    if ((ball->coords->y >= HEIGHT - ball->radius || ball->coords->y <= ball->radius) && fabs(ball->yvel) <= MIN_YVEL) ball->yvel = 0;
    else ball->yvel = (-1) * (ball->yvel * Y_DAMP_COEFF);

    if (ball->coords->y >= HEIGHT - ball->radius) ball->coords->y = HEIGHT - ball->radius;
    else ball->coords->y = ball->radius;

    ball->xvel = (ball->xvel * INVERSE_FRICTION_COEFF);
  }

  if (ball->coords->x >= WIDTH - ball->radius || ball->coords->x < ball->radius) {
    if (((ball->coords->x >= WIDTH - ball->radius || ball->coords->x <= ball->radius) && fabs(ball->xvel) <= MIN_XVEL)) ball->xvel = 0;
    else ball->xvel = (-1) * (ball->xvel * X_DAMP_COEFF);

    if (ball->coords-> x >= WIDTH - ball->radius) ball->coords->x = WIDTH - ball->radius;
    else ball->coords->x = ball->radius;
    printf("this is triggerred\n");
  }

  // printf("vvel: %f, y: %f\n", ball->yvel, ball->y);
}

void reflectionFrictionAndDamping(Circle** balls, int n){
  for (int i = 0; i < n; i++) reflectionFrictionAndDampingToBall(balls[i]);
}

void reInitiateMousePath(Circle* ball) {
  Point* tr = ball->path->st;
  while(tr) {
    Point* temp = tr;
    tr = tr->next;
    free(temp);
  }

  Point* newStart = createPoint(ball->coords->x, ball->coords->y);
  ball->path->st = newStart;
  ball->path->top = newStart;
  ball->path->n_points = 1;
}

void calculateTrajectory (Circle* ball, int* justReleasedMouse) {
  float xvel = 0, yvel = 0;
  int cnt = (TRAJECTORY_AVG_SIZE > ball->path->n_points) ? ball->path->n_points : TRAJECTORY_AVG_SIZE;
  if (cnt == 1) {
    ball->xvel = 0;
    ball->yvel = 0;
    *justReleasedMouse = 0;
    return;
  }

  float weight = TRAJECTORY_CALCULATION_WEIGHT, weightSum = 0;

  Point* tr = ball->path->top;
  while (tr->prev && cnt) {
    xvel += (tr->x - tr->prev->x) * weight;
    yvel += (tr->y - tr->prev->y) * weight;
    tr = tr->prev;
    weightSum += weight;
    weight += TRAJECTORY_CALCULATION_WEIGHT;
    cnt--;
  } 

  ball->xvel = (xvel/weightSum) * MOUSE_SENSITIVITY;
  ball->yvel = (yvel/weightSum) * MOUSE_SENSITIVITY;
  *justReleasedMouse = 0;
}

double calcPValue(Circle* ball, double x, double y) {
  double x_sq = (ball->coords->x - x), y_sq = (ball->coords->y - y), rad_sq = ball->radius;
  x_sq *= x_sq, y_sq *= y_sq, rad_sq *= rad_sq;

  double p_val = x_sq + y_sq - rad_sq;
  return p_val;
}

Circle* whichBallInBallInteraction(Circle** balls, int n, double x, double y) {
  for (int i = 0; i < n; i++) {
    if (calcPValue(balls[i], x, y) < 0) return balls[i];
  }

  return NULL;
}

// IMPLEMENT COLLISSION DETECTION

int main(int argc, char** argv) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Bouncy Ball Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // enterFullScreen(window);

  setRendererDrawColor(renderer, COLOR_BLACK);
  SDL_RenderClear(renderer);

  Circle** balls = createBalls(N_BALLS);

  drawBalls(renderer, balls, N_BALLS);
  SDL_RenderPresent(renderer);

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

        // case SDL_MOUSEBUTTONDOWN:
        //   mousePressed = 1;
        //   ball1->x = event.button.x;
        //   ball1->y = event.button.y;
        //   reInitiateMousePath(path1, ball1);
        //   break;
        //
        // case SDL_MOUSEBUTTONUP:
        //   mousePressed = 0;
        //   justReleasedMouse = 1;
        //   break;
        //
        // case SDL_MOUSEMOTION:
        //   if (mousePressed) {
        //     ball1->x = event.button.x;
        //     ball1->y = event.button.y;
        //   }
        //   break;
      }
    }

    setRendererDrawColor(renderer, COLOR_BLACK);
    SDL_RenderClear(renderer);

    // if (!mousePressed) applyGravity(ball1);
    applyGravity(balls, N_BALLS);

    // if (justReleasedMouse) calculateTrajectory(ball1, path1, &justReleasedMouse);

    reflectionFrictionAndDamping(balls, N_BALLS);

    nextPointsIntoPaths(balls, N_BALLS);
    // printf("%d\n", i++);
    drawPaths(renderer, balls, N_BALLS);

    drawBalls(renderer, balls, N_BALLS);
    SDL_RenderPresent(renderer);

    SDL_Delay(getFPS(SIMULATION_FPS));
  }
  
  deleteBalls(balls, N_BALLS);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}