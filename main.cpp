#include <windows.h>


int main() {
    

    while (true) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_MINIMIZE; // Open minimized

        if (CreateProcess(
                NULL,
                (LPSTR)"taskmgr.exe", // Program to execute
                NULL, NULL, FALSE,
                0, NULL, NULL,
                &si, &pi))
        {
            

            // Close handles to prevent leaks
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } 
        else {
            
        }

      
        Sleep(15000);
    }

    return 0;
}
