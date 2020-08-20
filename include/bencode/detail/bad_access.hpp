#pragma once
#include <stdexcept>

namespace bencode {

/// Base class for bad_bvalue_access and bad_bview_access exceptions.
class bad_access : public std::exception {
public:
    explicit bad_access(std::string_view msg)
        : what_(std::string(msg))
    {}

    const char* what() const noexcept override
    { return what_.c_str();}

private:
    std::string what_;
};

}