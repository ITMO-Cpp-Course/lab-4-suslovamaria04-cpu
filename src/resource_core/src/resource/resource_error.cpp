#include "lab4/resource/resource_error.hpp"

namespace lab4::resource {

ResourceError::ResourceError(const std::string& message) : message_(message) {}

const char* ResourceError::what() const noexcept { return message_.c_str(); }

}  // namespace lab4::resource