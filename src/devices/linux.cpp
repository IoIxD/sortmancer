#include <unistd.h>
#ifdef __linux__

#include "devices.hpp"
#include <blkid/blkid.h>
#include <filesystem>
#include <linux/limits.h>
#include <string.h>
#include <sys/mount.h>

std::vector<Device> Device::get() {
  std::vector<Device> devs;
  blkid_cache cache;

  blkid_get_cache(&cache, NULL);
  blkid_probe_all_removable(cache);

  auto iter = blkid_dev_iterate_begin(cache);
  blkid_dev dev;
  while (blkid_dev_next(iter, &dev) == 0) {
    Device d;
    memset(d.mLabel, 0, 255);
    memset(d.mDevName, 0, 255);
    memset(d.mFSType, 0, 255);
    d.mType = Type::Unknown;

    memcpy(d.mDevName, blkid_dev_devname(dev), PATH_MAX);
    memcpy(d.mUUID, blkid_dev_devname(dev), PATH_MAX);

    blkid_probe probe = blkid_new_probe_from_filename(d.mDevName);

    if (probe) {
      if (blkid_do_probe(probe) == 0) {
        const char *name;
        const char *data;
        size_t len;
        int nvals = blkid_probe_numof_values(probe);
        for (int n = 0; n < nvals; n++) {
          if (blkid_probe_get_value(probe, n, &name, &data, &len) == 0) {
            if (strcmp(name, "LABEL") == 0) {
              memcpy(d.mLabel, data, sizeof(d.mLabel) - 1);
            } else if (strcmp(name, "UUID") == 0) {
              memcpy(d.mUUID, data, strlen(data));
            } else if (strcmp(name, "TYPE") == 0) {
              memcpy(d.mFSType, data, strlen(data));
              if (strcmp(data, "iso9660") == 0) {
                d.mType = Type::CD;
              } else {
                printf("WARNING: %s has unknown type \"%s\"\n", d.mDevName,
                       d.mLabel);
              }
            }
          }
        }
      }
    }

    blkid_free_probe(probe);

    if (strcmp(d.mLabel, "") != 0) {
      devs.push_back(d);
    }
  }

  blkid_dev_iterate_end(iter);

  blkid_put_cache(cache);

  return devs;
};

std::optional<std::string> Device::mount() {
  std::filesystem::path path = mDevName;
  printf("mounting %s (%s)\n", mDevName, mFSType);
  char mountpoint[255];
  snprintf(mountpoint, 255, "/mnt/spcf/%s/", mUUID);
  if (!std::filesystem::exists(mountpoint)) {
    std::filesystem::create_directories(mountpoint);
  }
  auto err = ::mount(mDevName, mountpoint, mFSType,
                     MS_RDONLY | MS_NOEXEC | MS_NOATIME, NULL);
  /* ignore "device is busy" errors. */

  if (err != 0) {
    if (errno == EBUSY) {
      return mountpoint;
    } else {
      snprintf(this->mError, 255, "%s (Error %d)", strerror(errno), errno);
      return {};
    }
  }
  return mountpoint;
}

#endif
