#include "galena/window/win32/event_identifier.h"

#include <windows.h>


namespace galena {
namespace impl_win32 {


bool is_close_event(impl_win32::msg_type msg) {
    return msg == WM_CLOSE;
}


}
}
