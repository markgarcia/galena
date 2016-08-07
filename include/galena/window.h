#pragma once

#include "galena/window/window_event.h"

#include <functional>
#include <memory>
#include <string>


namespace galena {

namespace impl_win32 {

class window_impl;

}


class window {
public:
    window(int width, int height, const std::string& title = "");
    ~window();

    void set_event_processor(std::function<void(window_event&)> func);

    unsigned int get_width() const;
    unsigned int get_height() const;

    impl_win32::window_impl& impl() const;

private:
    std::unique_ptr<impl_win32::window_impl> window_impl;
};


}
