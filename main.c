#include "game.h"

int main() {
  initialize();
  setup();
  while (running) {
    process_user_input();
    update();
    render();
  }

  kill();
  return 0;
}
