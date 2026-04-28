#include "lab4/resource/file_handle.hpp"

#include "lab4/resource/resource_error.hpp"

namespace lab4::resource
{

FileHandle::FileHandle(const std::string& path) : file_(nullptr), path_(path), is_open_(false)
{
    file_ = fopen(path_.c_str(), "r+");
    if (!file_)
    {
        file_ = fopen(path_.c_str(), "w+");
        if (!file_)
        {
            throw ResourceError("Cannot open file: " + path_);
        }
    }
    is_open_ = true;
}

FileHandle::~FileHandle()
{
    if (file_)
    {
        fclose(file_);
        file_ = nullptr;
        is_open_ = false;
    }
}

FileHandle::FileHandle(FileHandle&& other) noexcept
    : file_(other.file_), path_(std::move(other.path_)), is_open_(other.is_open_)
{
    other.file_ = nullptr;
    other.is_open_ = false;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        if (file_)
        {
            fclose(file_);
        }
        file_ = other.file_;
        path_ = std::move(other.path_);
        is_open_ = other.is_open_;
        other.file_ = nullptr;
        other.is_open_ = false;
    }
    return *this;
}

std::string FileHandle::read() const
{
    check_open();

    long current_pos = ftell(file_);
    fseek(file_, 0, SEEK_END);
    long size = ftell(file_);
    fseek(file_, 0, SEEK_SET);

    std::string content(static_cast<size_t>(size), '\0');
    size_t bytes_read = fread(&content[0], 1, static_cast<size_t>(size), file_);
    if (bytes_read < static_cast<size_t>(size))
    {
        content.resize(bytes_read);
    }

    fseek(file_, current_pos, SEEK_SET);
    return content;
}

void FileHandle::write(const std::string& data)
{
    check_open();

    fclose(file_);
    file_ = fopen(path_.c_str(), "w");
    if (!file_)
    {
        throw ResourceError("Failed to reopen file: " + path_);
    }

    size_t written = fwrite(data.c_str(), 1, data.size(), file_);
    if (written != data.size())
    {
        throw ResourceError("Failed to write all data to: " + path_);
    }

    is_open_ = true;
    fflush(file_);
}

void FileHandle::append(const std::string& data)
{
    check_open();

    fseek(file_, 0, SEEK_END);
    size_t written = fwrite(data.c_str(), 1, data.size(), file_);

    if (written != data.size())
    {
        throw ResourceError("Failed to append all data to: " + path_);
    }

    fflush(file_);
}

bool FileHandle::is_open() const noexcept
{
    return is_open_;
}

const std::string& FileHandle::get_path() const noexcept
{
    return path_;
}

void FileHandle::check_open() const
{
    if (!is_open_)
    {
        throw ResourceError("File is not open: " + path_);
    }
}

} // namespace lab4::resource