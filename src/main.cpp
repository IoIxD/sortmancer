#include "gui/gui.hpp"
#include <Mw/Milsko.h>

int main() {
  MwLibraryInit();

  GUI g;

  MwLoop(g.window);
}
