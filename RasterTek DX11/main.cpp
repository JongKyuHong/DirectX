#include "systemclass.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    SystemClass* System;
    bool result;
    
    System = new SystemClass();
    result = System->Initialize();
    if (result)
    {
        System->Run();
    }
    
    System->Shutdown();
    delete System;
    System = 0;
    
    return 0;
}
