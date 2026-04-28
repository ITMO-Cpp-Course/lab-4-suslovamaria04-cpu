#pragma once

#include <cstdio>
#include <string>

namespace lab4::resource {

class FileHandle {
 public:
  explicit FileHandle(const std::string& path);

  ~FileHandle();

  FileHandle(const FileHandle&) = delete;
  FileHandle& operator=(const FileHandle&) = delete;

  FileHandle(FileHandle&& other) noexcept;
  FileHandle& operator=(FileHandle&& other) noexcept;

  std::string read() const;
  void write(const std::string& data);
  void append(const std::string& data);

  bool is_open() const noexcept;
  const std::string& get_path() const noexcept;

 private:
  FILE* file_;
  std::string path_;
  bool is_open_;

  void check_open() const;
};

}  // namespace lab4::resource