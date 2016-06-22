#pragma once

#include <cstdint>      // Seems to define __int64. We'll be fine, for now.


namespace galena {

namespace impl_win32 {

typedef unsigned int msg_type;

#if defined(_WIN64)
    typedef unsigned __int64 wparam_type;
#else
    typedef unsigned int wparam_type;
#endif

#if defined(_WIN64)
    typedef __int64 lparam_type;
#else
    typedef long lparam_type;
#endif

}

}
