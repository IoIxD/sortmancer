#include "../devices/devices.hpp"
#include "gui.hpp"

static void MWAPI ok(MwWidget handle, void *user, void *call) {
  (void)handle;
  (void)call;
  MwDestroyWidget(MwGetParent(handle));
}

static void device_activate(MwWidget handle, void *user, void *call) {
  GUI *self = (GUI *)user;
  char msg[256];
  MwWidget msgbox;

  auto name = MwListBoxGet(handle, *(int *)call);
  for (auto dev : self->scannedDevices) {
    if (dev.label() == std::string(name)) {
      printf("Mounting %s\n", dev.label().c_str());
      auto path = dev.mount();
      if (path) {
        self->start_scan(path.value());
        MwDestroyWidget(self->device_window);
      } else {
        MwWidget mb = MwMessageBox(handle, dev.error().c_str(), "Error",
                                   MwMB_ICONERROR | MwMB_BUTTONOK);
        MwAddUserHandler(MwMessageBoxGetChild(mb, MwMB_BUTTONOK),
                         MwNactivateHandler, ok, mb);
        MwReparent(mb, self->main_window);
      }
      return;
    }
  }
}

void GUI::device_choose_button_handler(MwWidget widget, void *user,
                                       void *client) {
  GUI *self = (GUI *)user;
  self->device_window =
      MwVaCreateWidget(MwWindowClass, "window", NULL, MwDEFAULT, MwDEFAULT, 320,
                       200, MwNtitle, "Select Device", NULL);

  MwReparent(self->device_window, self->main_window);

  self->device_listbox =
      MwCreateWidget(MwListBoxClass, "listbox", self->device_window, 15, 15,
                     320 - 32, 200 - 64);

  self->scannedDevices = Device::get();

  for (auto dev : self->scannedDevices) {
    if (dev.type() == Device::Type::Unknown) {
      continue;
    }
    printf("found %s\n", dev.label().c_str());
    MwListBoxSet(self->device_listbox, -1, 0, dev.label().c_str());
    MwAddUserHandler(self->device_listbox, MwNlistBoxActivateHandler,
                     device_activate, self);
  }

  self->device_window_browse =
      MwVaCreateWidget(MwButtonClass, "b", self->device_window, 320 - 15 - 75,
                       200 - 15 - 25, 75, 25, MwNtext, "Browse", NULL);
  MwAddUserHandler(self->device_window_browse, MwNactivateHandler,
                   file_choose_button_handler, self);
};
