#pragma once

#include <string>
#include <exception>

namespace lab4::resource {

class ResourceError : public std::exception {
public:
    explicit ResourceError(const std::string& message);
    const char* what() const noexcept override;
    
private:
    std::string message_;
};

} // namespace lab4::resource