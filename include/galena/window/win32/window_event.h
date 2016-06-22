#pragma once

#include "event_identifier.h"
#include "types.h"

#include <type_traits>
#include <utility>


namespace galena {

namespace impl_win32 {

    class window_impl;

}


class window_event {
    friend class impl_win32::window_impl;

    enum class result {
        none,
        prevent_close
    };

    impl_win32::msg_type msg;
    impl_win32::wparam_type wparam;
    impl_win32::lparam_type lparam;

    result result_value;

    bool is_processed;
    bool is_current_processed;

public:

    enum response {
        no_response,
        prevent_close
    };


    window_event(impl_win32::msg_type msg,
                 impl_win32::wparam_type wparam,
                 impl_win32::lparam_type lparam)
        : msg(msg), wparam(wparam), lparam(lparam), result_value(result::none),
          is_processed(false), is_current_processed(true) {}

    void not_processed() {
        is_current_processed = false;
    }

    template<typename func_type>
    void close(func_type&& func) {
        if(impl_win32::is_close_event(msg)) {
            auto response = call(std::forward<func_type>(func));
            if(response == prevent_close) result_value = result::prevent_close;
        }
    }


private:
    template<typename func_type, typename... arg_types>
    auto call(func_type&& func, arg_types... args)
    -> typename std::enable_if<std::is_void<decltype(std::forward<func_type>(func)(std::forward<arg_types>(args)...))>::value, response>::type {
        std::forward<func_type>(func)(std::forward<arg_types>(args)...);
        set_processed_state();
        return no_response;
    }

    template<typename func_type, typename... arg_types>
    auto call(func_type&& func, arg_types... args)
    -> typename std::enable_if<std::is_same<decltype(std::forward<func_type>(func)(std::forward<arg_types>(args)...)), response>::value, response>::type {
        auto response = std::forward<func_type>(func)(std::forward<arg_types>(args)...);
        set_processed_state();
        return response;
    }

    void set_processed_state() {
        if(is_current_processed) is_processed = true;
        is_current_processed = true;
    }
};


}
