#pragma once
#include "bencode/detail/exception.hpp"

namespace bencode {

class out_of_range : exception {
public:
    using exception::exception;
};

}