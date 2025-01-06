#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 70
#define PADDLE_VEL 300
#define PADDLE_HORIZONTAL_PADDING 5
#define PADDLE_VERTICAL_PADDING 2

#define BALL_WIDTH 20
#define BALL_HEIGHT 20
#define BALL_START_X SCR_WIDTH/2
#define BALL_START_Y SCR_HEIGHT/2
#define BALL_VEL_X 250
#define BALL_VEL_Y 250

typedef struct {
  float x;
  float y;
  float w;
  float h;
  float vel_x;
  float vel_y;
} GameObject;

int initialize(void);
void setup(void);
void process_user_input(void);
void update(void);
void render(void);
void kill(void);
void paddle_move_safe(GameObject* paddle, int direction);
void handle_point_event(int player);
void handle_paddle_movement_user(void);
void handle_paddle_movement_machine(void);
void handle_ball_movement(void);

extern GameObject ball;
extern GameObject paddle_left;
extern GameObject paddle_right;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern TTF_Font* font;
extern SDL_Color text_color;
extern bool running;
extern bool up_pressed;
extern bool down_pressed;
extern int points_player;
extern int points_machine;
extern char points_player_str[2];
extern char points_machine_str[2];
extern const unsigned char* keys;
extern int last_frame_time;
extern float time_delta;

#endif
