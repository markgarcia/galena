#include "galena/window.h"

#include "galena/window/win32/window_impl.h"


namespace galena {


window::window(int width, int height, const std::string& title)
    : window_impl(std::make_unique<impl_win32::window_impl>(width, height, title)) {}


window::~window() {}


void window::set_event_processor(std::function<void(window_event&)> func) {
    window_impl->set_event_processor(std::move(func));
}


impl_win32::window_impl& window::impl() const {
    return *window_impl;
}


unsigned int window::get_width() const {
    return window_impl->get_width();
}


unsigned int window::get_height() const {
    return window_impl->get_height();
}


}
