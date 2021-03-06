#pragma once

#include "galena/window.h"
#include "galena/renderer.h"
#include <memory>


namespace galena {

namespace impl {


class window_render_surface_impl {
public:
    virtual ~window_render_surface_impl() = 0;

    virtual void clear() = 0;
    virtual void present() = 0;

    virtual unsigned int get_width() const = 0;
    virtual unsigned int get_height() const = 0;
};


}


class window_render_surface {
public:
    window_render_surface(const window& window, renderer& renderer);

    void clear();
    void present();

    unsigned int get_width() const;
    unsigned int get_height() const;

    impl::window_render_surface_impl& get_impl() { return *m_impl; }

private:
    std::unique_ptr<impl::window_render_surface_impl> m_impl;
};


}
