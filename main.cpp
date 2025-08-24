#include <windows.h>
#include <iostream>

int main() {
    std::cout << "made by hawda.\n";
    std::cout << "24/8/2025.\n\n";

    while (true) {
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        // Try to launch Task Manager
        if (CreateProcess(
                NULL,
                (LPSTR)"taskmgr.exe", // Program to execute
                NULL, NULL, FALSE,
                0, NULL, NULL,
                &si, &pi))
        {
            std::cout << "fixing.. \n";

            // Close process and thread handles to avoid leaks
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } 
        else {
            std::cerr << "Error: " << GetLastError() << "\n";
        }

        // Wait 30 seconds before repeating
        Sleep(15000);
    }

    return 0;
}
