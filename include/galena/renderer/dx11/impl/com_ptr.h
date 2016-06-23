#pragma once

#include <memory>


class IUnknown;


namespace galena {
namespace impl {


template<typename T>
class com_ptr_deleter {
public:
    void operator()(T* obj) const {
        obj->Release();
    }
};


template<typename T>
using com_ptr = std::unique_ptr<T, com_ptr_deleter<T>>;


}
}