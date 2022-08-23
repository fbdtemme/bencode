#pragma once

#if defined(__GNUC__) && __GNUC__ < 12 && !defined(__llvm__)
    #define BENCODE_OLD_GCC_FORWARD_TEMPLATE_DECL
#endif
