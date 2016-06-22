#include "galena/application.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>


namespace galena {


void run() {
    BOOL ret;
    MSG msg;

    while((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if(ret == -1)
        {
            return;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


}
