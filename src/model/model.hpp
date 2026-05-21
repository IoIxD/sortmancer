#pragma once

#include <cstddef>
#include <cstring>
#include <string>
extern "C" {}

/* "/home/gavin/10kPhotos/Photos/Zimbabwe/p0306657.JPG" */

extern "C" {
struct model_context;
struct model_context *model_new(char *error_str, size_t error_str_len);
bool model_scan(struct model_context *model, const char *filename,
                char *error_str, size_t error_str_len);
void model_scanned_names_get_idx(struct model_context *model, size_t idx,
                                 char buf[255]);
}

class ModelContext {
  struct model_context *mInner;
  char mError[255];
  bool mValid = true;

public:
  ModelContext() {
    mInner = model_new(mError, 255);
    if (!mInner) {
      mValid = false;
    }
  }
  bool scan(std::string filename) {
    return model_scan(mInner, filename.c_str(), mError, 255);
  }
  void get_scanned_name(size_t idx, char out[255]) {
    model_scanned_names_get_idx(mInner, idx, out);
  }
  bool valid() { return mValid; }
  std::string error() { return mError; }
};
