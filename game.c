#include "game.h"

GameObject ball;
GameObject paddle_left;
GameObject paddle_right;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
SDL_Color text_color = {255, 255, 255, 255};
bool running = true;
bool up_pressed = false;
bool down_pressed = false;
int points_player = 0;
int points_machine = 0;
char points_player_str[2];
char points_machine_str[2];
const unsigned char* keys;
int last_frame_time = 0;
float time_delta;

int initialize(void) {  
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    puts("Error initializing SDL.");
    return 1;
  }

  if (TTF_Init() < 0) {
    puts("Error initializing TTF.");
    return 1;
  }
  
  window = SDL_CreateWindow("Pong",
			    SDL_WINDOWPOS_CENTERED,
			    SDL_WINDOWPOS_CENTERED,
			    SCR_WIDTH,
			    SCR_HEIGHT,
			    0);
  if (!window) {
    puts("Error initalizing window.");
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    puts("Error initializing renderer.");
    return 1;
  }

  font = TTF_OpenFont("/home/cenfra/Desktop/pong/Uknumberplate-A4Vx.ttf", 24);
  if (!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    return 1;
  }
  
  return 0;
}

// this function sets up game objects to their initial position and
// velocity values
void setup(void) {
  paddle_left.x = PADDLE_HORIZONTAL_PADDING;
  paddle_left.y = SCR_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  paddle_left.w = PADDLE_WIDTH;
  paddle_left.h = PADDLE_HEIGHT;
  paddle_left.vel_x = 0;
  paddle_left.vel_y = PADDLE_VEL;
  
  paddle_right.x = SCR_WIDTH - paddle_right.w - 2 * PADDLE_HORIZONTAL_PADDING;
  paddle_right.y = SCR_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  paddle_right.w = PADDLE_WIDTH;
  paddle_right.h = PADDLE_HEIGHT;
  paddle_right.vel_x = 0;
  paddle_right.vel_y = (int)PADDLE_VEL*0.87f;
  
  ball.x = BALL_START_X;
  ball.y = BALL_START_Y;
  ball.w = BALL_WIDTH;
  ball.h = BALL_HEIGHT;
  ball.vel_x = BALL_VEL_X;
  ball.vel_y = BALL_VEL_Y;
}

// this function moves the paddle within the top and bottom
// boundaries, or locks its position if its trying to move past the
// bounrary. 0 for downwards, 1 for upwards
void paddle_move_safe(GameObject* paddle, int direction) {
  switch (direction) {
  case 0:
    {
      int paddle_next_y = paddle->y + paddle->vel_y * time_delta;
      if (paddle_next_y + paddle->h <= SCR_HEIGHT) {
	paddle->y = paddle_next_y;
      }
    }
    break;
  case 1:
    {
      int paddle_next_y = paddle->y - paddle->vel_y * time_delta;
      if (paddle_next_y >= 0) {
	paddle->y = paddle_next_y;
      }
    }
    break;
  }
}

// this function handles the event of the player or machine scoring a
// point. 0 for machine point, 1 for player point.
void handle_point_event(int player) {
  switch (player) {
      case 0:
	points_machine += 1;
	break;
      case 1:
	points_player += 1;
    }

  // if score is 9, reset game
  if (points_player > 9 || points_machine > 9) {
    points_player = 0;
    points_machine = 0;
    setup();
  }

  ball.x = BALL_START_X;
  ball.y = BALL_START_Y;
  paddle_left.y = SCR_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  paddle_right.y = SCR_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  
}

// this function handles the left paddle movement (user)
void handle_paddle_movement_user(void) {
  if (up_pressed)
    paddle_move_safe(&paddle_left, 1);
  if (down_pressed)
    paddle_move_safe(&paddle_left, 0);
}

// this function (machine) moves the paddle on the right, trying to
// hit and bounce the ball away.
void handle_paddle_movement_machine(void) {
  if (ball.vel_x > 0 && ball.x > SCR_WIDTH * 0.53f) { // if ball is moving to right

    // if the ball is within the vertical paddle range, do nothing.
    // same procedure as checking vertical collision to test if ball
    // vertical coords are within range
    float range_paddle = paddle_right.y + paddle_right.h;
    bool top_ball_point_inside_range = (ball.y > paddle_right.y && ball.y <= range_paddle);
    bool bottom_ball_point_inside_range = (ball.y + ball.h > paddle_right.y && ball.y + ball.h <= range_paddle);
    bool within_range_vertical = (top_ball_point_inside_range || bottom_ball_point_inside_range);

    if (within_range_vertical)
      return;
    
    // tries to adjust right paddle vertical position based on the
    // ball's vertical position.
    if (paddle_right.y >= ball.y) {
      //paddle_right.y -= paddle_right.vel_y * time_delta;
      paddle_move_safe(&paddle_right, 1);
    } else {
      //paddle_right.y += paddle_right.vel_y * time_delta;
      paddle_move_safe(&paddle_right, 0);
    } 
  }  
}

// this function moves the ball and handles the cases in which the
// ball either collides with the vertical boundaries of the screen,
// collides with a paddle, or gets out of bounds.
void handle_ball_movement(void) {
  
  // calculate future position (in very next frame)
  float ball_next_x = ball.x + ball.vel_x * time_delta;
  float ball_next_y = ball.y + ball.vel_y * time_delta;

  // if ball hits vertical boundaries, invert y velocity.
  if (ball_next_y <= 0 || (ball_next_y + ball.h) >= SCR_HEIGHT) {
    ball.vel_y = -ball.vel_y;
    ball.y += ball.vel_y * time_delta;
  } else {
    ball.x = ball_next_x;
    ball.y = ball_next_y;
  }

  // now we check if the ball collides with paddles
  bool collision_horizontal, collision_vertical;

  if (ball.vel_x < 0) { // if moving to left
    // check if horizontal collision
    collision_horizontal = (ball_next_x <= paddle_left.x + paddle_left.w);
    // for the vertical colission, we need to check if either the top
    // or bottom point of the ball is within the range defined by the
    // height of the paddle.
    float range_paddle = paddle_left.y + paddle_left.h;
    //now we test the top and bottom points
    bool top_ball_point_inside_range = (ball_next_y > paddle_left.y && ball_next_y <= range_paddle);
    bool bottom_ball_point_inside_range = (ball_next_y + ball.h > paddle_left.y && ball_next_y + ball.h <= range_paddle);
    collision_vertical = (top_ball_point_inside_range || bottom_ball_point_inside_range);
  } else { // if moving to right
    // check if horizontal collision
    collision_horizontal = (ball_next_x + ball.w >= paddle_right.x);
    // for the vertical colission, we need to check if either the top
    // or bottom point of the ball is within the range defined by the
    // height of the paddle.
    float range_paddle = paddle_right.y + paddle_right.h;
    //now we test the top and bottom points
    bool top_ball_point_inside_range = (ball_next_y > paddle_right.y && ball_next_y <= range_paddle);
    bool bottom_ball_point_inside_range = (ball_next_y + ball.h > paddle_right.y && ball_next_y + ball.h <= range_paddle);
    collision_vertical = (top_ball_point_inside_range || bottom_ball_point_inside_range);
  }

  // if the ball collided with a paddle, invert horizontal movement
  // direction
  if (collision_horizontal && collision_vertical) {
    ball.vel_x = -ball.vel_x;
  }

  // now we check if the ball went out of boundaries
  if (ball_next_x + ball.w < 0) { // out of bounds on left side
    handle_point_event(0); // machine point
  } else if (ball_next_x > SCR_WIDTH) { // out of bounds on right side
    handle_point_event(1); // player point
  }
}

// this function updates the scene on each frame.
void update(void) {
  time_delta = (SDL_GetTicks() - last_frame_time) / 1000.0f;
  last_frame_time = SDL_GetTicks();
  // cap framerate
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    SDL_Delay(time_to_wait);

  handle_paddle_movement_user();
  handle_paddle_movement_machine();
  handle_ball_movement();
}

// this function renders the game objects each frame.
void render(void) {

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  /* ------------------ create rectangles ------------------ */

  SDL_Rect rect_paddle_left = {
    (int)paddle_left.x,
    (int)paddle_left.y,
    (int)paddle_left.w,
    (int)paddle_left.h,
  };

  SDL_Rect rect_paddle_right = {
    (int)paddle_right.x,
    (int)paddle_right.y,
    (int)paddle_right.w,
    (int)paddle_right.h,
  };

  SDL_Rect rect_ball = {
    (int)ball.x,
    (int)ball.y,
    (int)ball.w,
    (int)ball.h,
  };

  SDL_Rect rect_middle_line = {
    (int)(SCR_WIDTH / 2),
    0,
    10,
    (int)SCR_HEIGHT,
  };

  SDL_Rect rect_score_player = {
    (int)(SCR_WIDTH / 4),
    0,
    100,
    100,
  };

  SDL_Rect rect_score_machine = {
    (int)(SCR_WIDTH - (SCR_WIDTH / 4)) - 100,
    0,
    100,
    100,
  };

  /* ------------------------- draw ------------------------- */
  
  // draw middle line
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  SDL_RenderFillRect(renderer, &rect_middle_line);

  // convert integer to string
  sprintf(points_player_str, "%d", points_player);
  sprintf(points_machine_str, "%d", points_machine);

  // create score number textures
  SDL_Surface* surface_score;
  surface_score = TTF_RenderText_Solid(font, points_player_str, text_color);
  SDL_Texture* texture_score_player = SDL_CreateTextureFromSurface(renderer, surface_score);
  surface_score = TTF_RenderText_Solid(font, points_machine_str, text_color);
  SDL_Texture* texture_score_machine = SDL_CreateTextureFromSurface(renderer, surface_score);
  SDL_FreeSurface(surface_score);

  // display textures
  SDL_RenderCopy(renderer, texture_score_player, NULL, &rect_score_player);
  SDL_RenderCopy(renderer, texture_score_machine, NULL, &rect_score_machine);

  // draw paddles
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &rect_paddle_left);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &rect_paddle_right);

  // draw ball
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &rect_ball);
  
  // blit
  SDL_RenderPresent(renderer);
}

void kill(void) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

// handles basic user input events, 'w' or 'p' for moving left paddle
// upwards and 's' or 'n' for moving paddle downwards.
void process_user_input(void) {
  
  SDL_Event e;

  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      running = false;
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {
      case SDLK_q:
	running = false;
	break;
      case SDLK_w:
      case SDLK_p:
	up_pressed = true;
	break;
      case SDLK_s:
      case SDLK_n:
	down_pressed = true;
	break;
      }
      break;
    case SDL_KEYUP:
      switch (e.key.keysym.sym) {
      case SDLK_w:
      case SDLK_p:
	up_pressed = false;
	break;
      case SDLK_s:
      case SDLK_n:
	down_pressed = false;
	break;
      }
    }
  }
}
