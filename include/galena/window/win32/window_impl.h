#pragma once

#include "window_event.h"
#include "types.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <memory>
#include <string>


namespace galena {
namespace impl_win32 {


class window_impl {
public:
    window_impl(int width, int height, const std::string& title);
    ~window_impl();

    HWND native_handle() { return hwnd; }

    void set_event_processor(std::function<void(window_event&)> func);

    window_event::result process_event(msg_type msg, wparam_type wparam, lparam_type lparam);
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

private:
    HWND hwnd = NULL;
    std::function<void(window_event&)> event_processor;
};


}
}
