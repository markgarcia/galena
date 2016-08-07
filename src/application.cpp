#include "galena/application.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>


namespace galena {


void application::process_events() {
    MSG msg = {};
    while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if(msg.message == WM_QUIT) {
            m_is_open = false;
        }
    }
}


}
