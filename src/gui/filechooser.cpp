#include "gui.hpp"
#include <algorithm>

static void MWAPI file_callback(MwWidget handle, void *user_data,
                                void *call_data) {
  GUI *self = (GUI *)user_data;

  self->activateScanner = 1;

  self->start_scan((char *)call_data);

  MwDispatchUserHandler(handle, MwNcloseHandler, NULL);
}

void GUI::file_choose_button_handler(MwWidget widget, void *user,
                                     void *client) {
  GUI *self = (GUI *)user;

  self->directory_chooser =
      MwDirectoryChooser(self->main_window, "Scan a Directory");
  MwAddUserHandler(self->directory_chooser, MwNdirectoryChosenHandler,
                   file_callback, self);
};

void GUI::window_scan_thing(MwWidget widget, void *user, void *client) {
  GUI *self = (GUI *)user;
  bool didScanBoxCreate = false;
  self->scanMutex.lock();
  for (auto sc : self->scanBoxCreationQueue) {
    if (sc.idx >= self->scanLines.size()) {
      self->scanLines.resize(sc.idx + 1);
    }
    self->scanLines[sc.idx].tab = MwTabAdd(self->tab_view, sc.dir);

    int width = MwGetInteger(self->scanLines[sc.idx].tab, MwNwidth);
    int height = MwGetInteger(self->scanLines[sc.idx].tab, MwNheight);
    MwWidget box =
        MwVaCreateWidget(MwListBoxClass, "box", self->scanLines[sc.idx].tab, 0,
                         0, width - 1, height - 1, NULL);
    MwListBoxSetWidth(box, 0, -384);
    int index = MwListBoxSet(box, -1, 0, "Filename");
    MwListBoxSet(box, index, -1, "Keywords");

    self->scanLines[sc.idx].box = box;

    didScanBoxCreate = true;
  }
  if (didScanBoxCreate) {
    self->scanBoxCreationQueue.erase(self->scanBoxCreationQueue.begin());
  }

  bool didScanEntryCreate = false;
  for (auto sc : self->scanBoxEntryQueue) {
    int index = MwListBoxSet(self->scanLines[sc.idx].box, -1, 0, sc.line1);
    MwListBoxSet(self->scanLines[sc.idx].box, index, -1, sc.line2);

    didScanEntryCreate = true;
  }
  if (didScanEntryCreate) {
    self->scanBoxEntryQueue.erase(self->scanBoxEntryQueue.begin());
  }
  self->scanMutex.unlock();
}
