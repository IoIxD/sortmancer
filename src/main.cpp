#include "gui/gui.hpp"
#include <Mw/Milsko.h>

int main() {
  // if (getuid() != 0) {
  //   printf("Program must be run as root.\n");
  // }
  MwLibraryInit();

  GUI g;

  MwLoop(g.main_window);
}
