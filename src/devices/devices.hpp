#pragma once

#include <linux/limits.h>
#include <optional>
#include <string>
#include <vector>

class Device {
public:
  enum class Type { CD = 0, Unknown };
  char mError[256];

private:
  char mLabel[PATH_MAX + 1];
  Type mType;

#ifdef __linux__
  char mDevName[PATH_MAX + 1];
  char mUUID[PATH_MAX + 1];
  char mFSType[255];
#endif

public:
  static std::vector<Device> get();

  std::string label() { return mLabel; };
  Device::Type type() { return mType; }

  std::optional<std::string> mount();
  void unmount();
  std::string error() { return mError; };
};
