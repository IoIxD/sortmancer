#include "gui.hpp"

static void MWAPI ok(MwWidget handle, void *user, void *call) {
  (void)handle;
  (void)call;
  MwDestroyWidget(MwGetParent(handle));
}

void GUI::MainWindow::search_btn(MwWidget handle, void *user_data,
                                 void *call_data) {
  GUI::MainWindow *self = (GUI::MainWindow *)user_data;

  const char *text = MwGetText(self->search_box, MwNtext);

  if (text) {
    if (self->search_results_listbox) {
      MwDestroyWidget(self->search_results_listbox);
    }
    int width = MwGetInteger(self->search_results_box, MwNwidth);
    int height = MwGetInteger(self->search_results_box, MwNheight);

    self->search_results_listbox =
        MwCreateWidget(MwListBoxClass, "", self->search_results_box, 0, 0,
                       width - 1, height - 1);
    auto entries = self->gui->db.search(text, NULL, self);

    MwListBoxSetWidth(self->search_results_listbox, 0, (width / 3));
    MwListBoxSetWidth(self->search_results_listbox, 1, (width / 3));
    MwListBoxSetWidth(self->search_results_listbox, 2, (width / 3));

    int index = MwListBoxSet(self->search_results_listbox, -1, 0, "Location");
    MwListBoxSet(self->search_results_listbox, index, -1, "Filename");
    MwListBoxSet(self->search_results_listbox, index, -1, "Matched Keywords");

    for (auto entry : entries) {
      int index = MwListBoxSet(self->search_results_listbox, -1, 0,
                               entry.table_name.c_str());
      index = MwListBoxSet(self->search_results_listbox, index, -1,
                           entry.filename.c_str());
      MwListBoxSet(self->search_results_listbox, index, -1,
                   entry.keywords.c_str());
    }
  }
};

GUI::MainWindow::MainWindow(GUI *gui) : gui(gui) {
  main_window =
      MwVaCreateWidget(MwWindowClass, "main", NULL, MwDEFAULT, MwDEFAULT, 640,
                       480, MwNtitle, "Sortmancer", NULL);
  main_box = MwVaCreateWidget(MwBoxClass, "box", main_window, 0, 0, 640, 480,
                              MwNorientation, MwVERTICAL, MwNpadding, 16, NULL);
  search_box_holder = MwVaCreateWidget(MwBoxClass, "entry_box", main_box, 0, 0,
                                       640, 16, MwNratio, 1, NULL);
  device_scan_button_holder = MwVaCreateWidget(
      MwBoxClass, "entry_box", main_box, 0, 0, 640, 16, MwNratio, 1, NULL);
  device_scan_button =
      MwVaCreateWidget(MwButtonClass, "entry", device_scan_button_holder, 0, 0,
                       640, 16, MwNratio, 8, MwNtext, "Scan", NULL);

  search_box_text =
      MwVaCreateWidget(MwLabelClass, "entry_text", search_box_holder, 0, 0, 1,
                       1, MwNtext, "Search", MwNratio, 1, NULL);
  search_box = MwVaCreateWidget(MwEntryClass, "entry", search_box_holder, 0, 0,
                                640, 16, MwNratio, 8, NULL);
  search_box_button =
      MwVaCreateWidget(MwButtonClass, "entry", search_box_holder, 0, 0, 640, 16,
                       MwNratio, 1, MwNtext, "Go", NULL);
  tab_view = MwVaCreateWidget(MwTabClass, "entry", main_box, 0, 0, 640, 16,
                              MwNratio, 16, NULL);
  search_results_box = MwTabAdd(tab_view, "Results");

  /* TODO: these should be moved to be within GUI::MainWindow, but testing it
   * properly would require doing *another* scan, even partially, and I don't
   * want to fuck with this. */
  MwAddUserHandler(device_scan_button, MwNactivateHandler,
                   device_choose_button_handler, gui);
  MwAddUserHandler(main_window, MwNtickHandler, window_tick, gui);

  MwAddUserHandler(main_window, MwNresizeHandler, resize, this);
  MwAddUserHandler(search_box_button, MwNactivateHandler, search_btn, this);
}

void GUI::MainWindow::resize(MwWidget handle, void *user_data,
                             void *call_data) {
  GUI::MainWindow *self = (GUI::MainWindow *)user_data;
  unsigned int w, h, mh;

  (void)user_data;
  (void)call_data;

  w = MwGetInteger(handle, MwNwidth);
  h = MwGetInteger(handle, MwNheight);

  MwVaApply(self->main_box, MwNx, 0, MwNy, 0, MwNwidth, w, MwNheight, h, NULL);

  if (self->search_results_listbox) {
    int width = MwGetInteger(self->search_results_box, MwNwidth);
    int height = MwGetInteger(self->search_results_box, MwNheight);

    MwVaApply(self->search_results_listbox, MwNwidth, width, MwNheight, height,
              NULL);
    MwListBoxSetWidth(self->search_results_listbox, 0, (width / 3));
    MwListBoxSetWidth(self->search_results_listbox, 1, (width / 3));
    MwListBoxSetWidth(self->search_results_listbox, 2, (width / 3));
  }

  for (auto sl : self->gui->scanLines) {
    auto width = MwGetInteger(sl.tab, MwNwidth);
    auto height = MwGetInteger(sl.tab, MwNheight);

    MwVaApply(sl.box, MwNx, 0, MwNy, 0, MwNwidth, width, MwNheight, height,
              NULL);
    MwListBoxSetWidth(sl.tab, 0, (width / 2));
    MwListBoxSetWidth(sl.tab, 1, (width / 2));
    MwDispatchUserHandler(sl.box, MwNresizeHandler, sl.box);
  };
};

void GUI::MainWindow::window_tick(MwWidget widget, void *user, void *client) {
  GUI *self = (GUI *)user;

  bool didErrorCreate = false;
  bool didSBCCreate = false;
  bool didSBECreate = false;
  bool didScanStartCreate = false;

  self->errMutex.lock();
  for (auto err : self->errorCreationQueue) {
    MwWidget mb = MwMessageBox(widget, err.c_str(), "Error",
                               MwMB_ICONERROR | MwMB_BUTTONOK);
    MwAddUserHandler(MwMessageBoxGetChild(mb, MwMB_BUTTONOK),
                     MwNactivateHandler, ok, mb);
    MwReparent(mb, self->main_window->main_window);
    didErrorCreate = true;
  }
  self->errMutex.unlock();
  if (didErrorCreate)
    self->errorCreationQueue.erase(self->errorCreationQueue.begin());

  for (auto sc : self->scanBoxCreationQueue) {
    if (sc.idx >= self->scanLines.size()) {
      self->scanLines.resize(sc.idx + 1);
    }
    self->scanLines[sc.idx].tab = MwTabAdd(self->main_window->tab_view, sc.dir);
    MwTabFocus(self->main_window->tab_view, sc.dir);

    int width = MwGetInteger(self->scanLines[sc.idx].tab, MwNwidth);
    int height = MwGetInteger(self->scanLines[sc.idx].tab, MwNheight);
    MwWidget box =
        MwVaCreateWidget(MwListBoxClass, "box", self->scanLines[sc.idx].tab, 0,
                         0, width - 1, height - 1, NULL);
    MwListBoxSetWidth(box, 0, -384);
    int index = MwListBoxSet(box, -1, 0, "Filename");
    MwListBoxSet(box, index, -1, "Keywords");

    self->scanLines[sc.idx].box = box;
    didSBCCreate = true;
  }
  if (didSBCCreate)
    self->scanBoxCreationQueue.erase(self->scanBoxCreationQueue.begin());

  self->tickMutex.lock();
  for (auto sc : self->scanBoxEntryQueue) {
    int index = MwListBoxSet(self->scanLines[sc.idx].box, -1, 0, sc.line1);
    MwListBoxSet(self->scanLines[sc.idx].box, index, -1, sc.line2);
    didSBECreate = true;
  }
  if (didSBECreate)
    self->scanBoxEntryQueue.erase(self->scanBoxEntryQueue.begin());
  self->tickMutex.unlock();

  for (auto scan : self->scanStartQueue) {
    self->start_scan(scan.dir, scan.labelName);
    didScanStartCreate = true;
  }
  if (didScanStartCreate)
    self->scanStartQueue.erase(self->scanStartQueue.begin());
}

GUI::MainWindow::~MainWindow() {
  MwDestroyWidget(main_window);
  MwDestroyWidget(main_box);
  MwDestroyWidget(tab_view);
  MwDestroyWidget(search_results_box);
  MwDestroyWidget(search_box_holder);
  MwDestroyWidget(search_box_text);
  MwDestroyWidget(search_box);
  MwDestroyWidget(search_box_button);
  MwDestroyWidget(device_scan_button_holder);
  MwDestroyWidget(device_scan_button);
  MwDestroyWidget(directory_chooser);
}
