#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "file_handle.hpp"

namespace lab4::resource {

class ResourceManager {
 public:
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager& operator=(const ResourceManager&) = delete;

  static ResourceManager& instance();

  std::shared_ptr<FileHandle> open_file(const std::string& path);

  void close_file(const std::string& path);

  void cleanup();

 private:
  ResourceManager() = default;

  std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};

}  // namespace lab4::resource