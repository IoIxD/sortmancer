#include "gui.hpp"
#include <algorithm>
void GUI::dir_recurse(const std::string &path,
                      std::function<void(const std::filesystem::path &)> cb) {
  if (auto dir = opendir(path.c_str())) {
    while (auto f = readdir(dir)) {
      if (f->d_name[0] == '.')
        continue;
      struct stat buf;
      lstat((path + f->d_name).c_str(), &buf);
      if (S_ISDIR(buf.st_mode)) {
        dir_recurse(path + std::filesystem::path::preferred_separator +
                        f->d_name + std::filesystem::path::preferred_separator,
                    cb);
      } else {
        // printf(">%s\n", f->d_name);
        cb(path + f->d_name);
      }
    }
    closedir(dir);
  }
};

GUI::GUI() {
  main_window = MwVaCreateWidget(MwWindowClass, "main", NULL, MwDEFAULT,
                                 MwDEFAULT, 640, 480, MwNtitle, "SPCF", NULL);

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
  tab_view = MwVaCreateWidget(MwTabClass, "entry", main_box, 0, 0, 640, 16,
                              MwNratio, 16, NULL);
  search_results_box = MwTabAdd(tab_view, "Results");

  MwAddUserHandler(device_scan_button, MwNactivateHandler,
                   device_choose_button_handler, this);
  MwAddUserHandler(main_window, MwNtickHandler, window_scan_thing, this);
}

void GUI::start_scan(std::string dir) {
  if (!modelContext) {
    modelContext = new ModelContext();
  }
  GUI::ScanCreationEntry creationEntry;
  this->showingScan = true;

  int i = this->scanBoxEntryQueue.size();
  creationEntry.idx = i;
  memcpy(creationEntry.dir, dir.c_str(), 255);

  this->scanBoxCreationQueue.push_back(creationEntry);

  printf("starting scan of %s\n", dir.c_str());

  this->scanThreads.push_back(new std::thread([=]() {
    this->dir_recurse(creationEntry.dir, [=](std::filesystem::path path) {
      printf("%s\n", path.c_str());
      auto e = path.extension().string();
      std::transform(e.begin(), e.end(), e.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      for (auto ext : avail_file_exts) {
        if (e == ext) {
          printf(">%s\n", path.filename().c_str());
          this->modelContext->scan(path.string().c_str());
          std::string foundLabels = "";

          for (int i = 0; i < 32; i++) {
            char name[255] = {0};
            this->modelContext->get_scanned_name(i, name);
            foundLabels += name;
            foundLabels += ", ";
          }
          GUI::ScanEntry entry = {
              .idx = i,
          };
          snprintf(entry.line1, 255, "%s", path.filename().c_str());
          snprintf(entry.line2, 255, "%s", foundLabels.c_str());

          this->scanMutex.lock();
          this->scanBoxEntryQueue.push_back(entry);
          this->scanMutex.unlock();

          break;
        };
      }
    });
  }));
  this->activateScanner = 1;
}
