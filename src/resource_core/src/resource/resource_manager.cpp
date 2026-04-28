#include "lab4/resource/resource_manager.hpp"
#include "lab4/resource/resource_error.hpp"
namespace lab4::resource
{

ResourceManager& ResourceManager::instance()
{
    static ResourceManager instance;
    return instance;
}

std::shared_ptr<FileHandle> ResourceManager::open_file(const std::string& path)
{
    auto it = cache_.find(path);
    if (it != cache_.end())
    {
        auto ptr = it->second.lock();
        if (ptr)
        {
            return ptr;
        }
        else
        {
            cache_.erase(it);
        }
    }

    auto new_ptr = std::make_shared<FileHandle>(path);
    cache_[path] = new_ptr;
    return new_ptr;
}

void ResourceManager::close_file(const std::string& path)
{
    cache_.erase(path);
}

void ResourceManager::cleanup()
{
    for (auto it = cache_.begin(); it != cache_.end();)
    {
        if (it->second.expired())
        {
            it = cache_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace lab4::resource