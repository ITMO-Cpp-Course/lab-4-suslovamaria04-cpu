#include <catch2/catch_all.hpp>
#include <cstring>
#include <filesystem>
#include <vector>

#include "lab4/resource/file_handle.hpp"
#include "lab4/resource/resource_error.hpp"
#include "lab4/resource/resource_manager.hpp"

namespace fs = std::filesystem;
using namespace lab4::resource;

class TempFile
{
  public:
    TempFile() : path_("test_" + std::to_string(rand()) + ".txt") {}
    ~TempFile()
    {
        if (fs::exists(path_))
            fs::remove(path_);
    }
    const std::string& path() const
    {
        return path_;
    }

  private:
    std::string path_;
};

//  FileHandle tests

TEST_CASE("FileHandle creates file", "[FileHandle]")
{
    TempFile tf;
    REQUIRE_FALSE(fs::exists(tf.path()));
    {
        FileHandle fh(tf.path());
        REQUIRE(fh.is_open());
    }
    REQUIRE(fs::exists(tf.path()));
}

TEST_CASE("FileHandle write/read", "[FileHandle]")
{
    TempFile tf;
    FileHandle fh(tf.path());
    fh.write("Hello");
    REQUIRE(fh.read() == "Hello");
    fh.write("Overwrite");
    REQUIRE(fh.read() == "Overwrite");
}

TEST_CASE("FileHandle append", "[FileHandle]")
{
    TempFile tf;
    FileHandle fh(tf.path());
    fh.write("Hello");
    fh.append(" World");
    REQUIRE(fh.read() == "Hello World");
}

TEST_CASE("FileHandle read preserves position", "[FileHandle]")
{
    TempFile tf;
    FileHandle fh(tf.path());
    fh.write("12345");
    REQUIRE(fh.read() == "12345");
    REQUIRE(fh.read() == "12345");
}

TEST_CASE("FileHandle move semantics", "[FileHandle]")
{
    TempFile tf;
    FileHandle orig(tf.path());
    orig.write("data");
    FileHandle moved(std::move(orig));
    REQUIRE_FALSE(orig.is_open());
    REQUIRE(moved.is_open());
    REQUIRE(moved.read() == "data");

    TempFile tf2;
    FileHandle other(tf2.path());
    other = std::move(moved);
    REQUIRE_FALSE(moved.is_open());
    REQUIRE(other.read() == "data");
}

TEST_CASE("FileHandle throws on closed file", "[FileHandle]")
{
    TempFile tf;
    FileHandle orig(tf.path());
    FileHandle moved(std::move(orig));
    REQUIRE_THROWS_AS(orig.read(), ResourceError);
    REQUIRE_THROWS_AS(orig.write("x"), ResourceError);
    REQUIRE_THROWS_AS(orig.append("x"), ResourceError);
}

TEST_CASE("FileHandle empty and large data", "[FileHandle]")
{
    TempFile tf;
    FileHandle fh(tf.path());
    fh.write("");
    REQUIRE(fh.read().empty());

    std::string big(1024 * 1024, 'A');
    fh.write(big);
    REQUIRE(fh.read().size() == big.size());
}

TEST_CASE("FileHandle binary data", "[FileHandle]")
{
    TempFile tf;
    FileHandle fh(tf.path());
    std::vector<unsigned char> bin = {0x00, 0xFF, 0x7E};
    std::string data(bin.begin(), bin.end());
    fh.write(data);
    std::string read = fh.read();
    REQUIRE(read.size() == bin.size());
    REQUIRE(std::memcmp(read.data(), bin.data(), bin.size()) == 0);
}

//  ResourceManager tests

TEST_CASE("ResourceManager singleton", "[ResourceManager]")
{
    auto& rm1 = ResourceManager::instance();
    auto& rm2 = ResourceManager::instance();
    REQUIRE(&rm1 == &rm2);
}

TEST_CASE("ResourceManager caches resources", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto p1 = rm.open_file(tf.path());
    auto p2 = rm.open_file(tf.path());
    REQUIRE(p1.get() == p2.get());
    REQUIRE(p1.use_count() == 2);
}

TEST_CASE("ResourceManager different paths", "[ResourceManager]")
{
    TempFile tf1, tf2;
    auto& rm = ResourceManager::instance();
    auto p1 = rm.open_file(tf1.path());
    auto p2 = rm.open_file(tf2.path());
    REQUIRE(p1.get() != p2.get());
}

TEST_CASE("ResourceManager cleanup expired", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    std::weak_ptr<FileHandle> wp;
    {
        auto p = rm.open_file(tf.path());
        wp = p;
        REQUIRE_FALSE(wp.expired());
    }
    rm.cleanup();
    REQUIRE(wp.expired());
}

TEST_CASE("ResourceManager manual close", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto p = rm.open_file(tf.path());
    auto* raw = p.get();
    rm.close_file(tf.path());
    auto p2 = rm.open_file(tf.path());
    REQUIRE(p2.get() != raw);
}

TEST_CASE("ResourceManager shared ownership", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto p1 = rm.open_file(tf.path());
    auto p2 = p1;
    p1->write("shared");
    REQUIRE(p2->read() == "shared");
    REQUIRE(p1.use_count() == 2);
}

TEST_CASE("ResourceManager file operations", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto h = rm.open_file(tf.path());
    h->write("Hello");
    h->append(" World");
    REQUIRE(h->read() == "Hello World");
}

TEST_CASE("ResourceManager persistence after shared_ptr destruction", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    {
        auto p = rm.open_file(tf.path());
        p->write("persistent");
    }
    auto p2 = rm.open_file(tf.path());
    REQUIRE(p2->read() == "persistent");
}

TEST_CASE("ResourceManager multiple opens and cleanup", "[ResourceManager]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    std::vector<std::shared_ptr<FileHandle>> vec;
    for (int i = 0; i < 5; ++i)
        vec.push_back(rm.open_file(tf.path()));
    REQUIRE(vec[0].use_count() == 5);
    vec.clear();
    rm.cleanup();
    auto p = rm.open_file(tf.path());
    REQUIRE(p.use_count() == 1);
}

// Integration tests

TEST_CASE("Integration: full workflow", "[Integration]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto h1 = rm.open_file(tf.path());
    h1->write("Init");
    auto h2 = rm.open_file(tf.path());
    h2->append(" + appended");
    REQUIRE(h1->read() == "Init + appended");
    h1->write("New");
    REQUIRE(h2->read() == "New");
}

TEST_CASE("Integration: multiple files", "[Integration]")
{
    std::vector<TempFile> tfs(3);
    auto& rm = ResourceManager::instance();
    std::vector<std::shared_ptr<FileHandle>> handles;
    for (auto& tf : tfs)
        handles.push_back(rm.open_file(tf.path()));
    for (size_t i = 0; i < handles.size(); ++i)
        handles[i]->write("File" + std::to_string(i));
    for (size_t i = 0; i < handles.size(); ++i)
        REQUIRE(handles[i]->read() == "File" + std::to_string(i));
}

TEST_CASE("Integration: move with manager", "[Integration]")
{
    TempFile tf;
    auto& rm = ResourceManager::instance();
    auto ptr = rm.open_file(tf.path());
    ptr->write("move");
    FileHandle moved(std::move(*ptr));
    REQUIRE_FALSE(ptr->is_open());
    REQUIRE(moved.read() == "move");
}

TEST_CASE("Integration: exception on invalid path", "[Integration]")
{
    REQUIRE_THROWS_AS(FileHandle("/nonexistent_dir_12345/file.txt"), ResourceError);
}