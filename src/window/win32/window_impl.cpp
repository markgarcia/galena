#include "galena/window/win32/window_impl.h"

#include <stdexcept>
#include <memory>


namespace galena {
namespace impl_win32 {


std::wstring str_to_ws(const std::string& str)
{
    auto ws_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size() + 1), nullptr, 0);

    auto buffer = std::make_unique<wchar_t[]>(static_cast<std::size_t>(ws_len));
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), buffer.get(), ws_len);
    return { buffer.get() };
}


window_impl::window_impl(int width, int height, const std::string& title) {
    auto hinstance = GetModuleHandle(0);

    auto ws_title = str_to_ws(title);

    WNDCLASSEX window_class;
    if(!GetClassInfoEx(hinstance, L"genui_nativity_window", &window_class)) {
        window_class = {};
        window_class.cbSize = sizeof(WNDCLASSEX);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = wnd_proc;
        window_class.hInstance = hinstance;
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        window_class.lpszClassName = L"genui_nativity_window";
        if(!RegisterClassEx(&window_class)) throw std::runtime_error("Unable to create window class.");
    }

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, FALSE );
    hwnd = CreateWindowEx(0, window_class.lpszClassName, ws_title.c_str(),
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                            rect.right - rect.left, rect.bottom - rect.top,
                            NULL, NULL, hinstance,
                            reinterpret_cast<void*>(this));
    if(!hwnd) throw std::runtime_error("Unable to create window.");
}


window_impl::~window_impl() {
    if(hwnd) DestroyWindow(hwnd);
}


void window_impl::set_event_processor(std::function<void(window_event&)> func) {
    event_processor = std::move(func);
}


unsigned int window_impl::get_width() const {
    RECT rect;
    GetClientRect(hwnd, &rect);
    return rect.right - rect.left;
}


unsigned int window_impl::get_height() const {
    RECT rect;
    GetClientRect(hwnd, &rect);
    return rect.bottom - rect.top;
}


window_event::result window_impl::process_event(msg_type msg, wparam_type wparam, lparam_type lparam) {
    if(!event_processor) return window_event::result::none;

    window_event event{ msg, wparam, lparam };
    event_processor(event);

    return event.result_value;
}


LRESULT CALLBACK window_impl::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    auto window = reinterpret_cast<window_impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch(message) {
    case WM_CREATE:
        {
            auto createstruct_ptr = reinterpret_cast<LPCREATESTRUCT>(lparam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createstruct_ptr->lpCreateParams));
        }

        return 0;

    case WM_DESTROY:
        window->hwnd = NULL;
        PostQuitMessage(0);
        return 0;

    default:
        if(!window) {
            return DefWindowProc(hwnd, message, wparam, lparam);
        }

        auto result = window->process_event(message, wparam, lparam);
        if(result == window_event::result::none) {
            return DefWindowProc(hwnd, message, wparam, lparam);
        }
        else if(result == window_event::result::prevent_close) {
            return 0;
        }
        else {
            return DefWindowProc(hwnd, message, wparam, lparam);
        }
    }
}


}
}
