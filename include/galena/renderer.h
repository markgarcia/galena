#pragma once

#include <memory>


namespace galena {


class renderer {
public:
    enum class renderer_type {
        dx11
    };

    virtual ~renderer() = 0;

    static std::unique_ptr<renderer> create(renderer_type type);
};


}