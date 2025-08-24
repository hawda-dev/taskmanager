#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

struct FindByPid {
    DWORD pid;
    HWND hwnd;
};

static BOOL CALLBACK EnumWinByPidProc(HWND hwnd, LPARAM lParam) {
    FindByPid* data = reinterpret_cast<FindByPid*>(lParam);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == data->pid && GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd)) {
        data->hwnd = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND GetMainWindowFromPid(DWORD pid) {
    FindByPid data{ pid, NULL };
    EnumWindows(EnumWinByPidProc, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

HWND FindTaskManagerWindow() {
    if (HWND h = FindWindowW(L"TaskManagerWindow", NULL)) return h;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return NULL;

    PROCESSENTRY32W pe{ sizeof(pe) };
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (_wcsicmp(pe.szExeFile, L"Taskmgr.exe") == 0) {
            CloseHandle(snap);
            return GetMainWindowFromPid(pe.th32ProcessID);
        }
    }
    CloseHandle(snap);
    return NULL;
}

bool IsForegroundFullscreen() {
    HWND fg = GetForegroundWindow();
    if (!fg) return false;

    RECT r{};
    if (FAILED(DwmGetWindowAttribute(fg, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) {
        if (!GetWindowRect(fg, &r)) return false;
    }

    HMONITOR mon = MonitorFromWindow(fg, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi{ sizeof(mi) };
    if (!GetMonitorInfo(mon, &mi)) return false;

    const int tol = 2;
    return (
        r.left   <= mi.rcMonitor.left  + tol &&
        r.top    <= mi.rcMonitor.top   + tol &&
        r.right  >= mi.rcMonitor.right - tol &&
        r.bottom >= mi.rcMonitor.bottom- tol
    );
}

void MinimizeTaskManagerNoActivate() {
    if (HWND tm = FindTaskManagerWindow()) {
        ShowWindowAsync(tm, SW_SHOWMINNOACTIVE);
        ShowWindowAsync(tm, SW_MINIMIZE);
        return;
    }

    if (IsForegroundFullscreen()) return;

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINNOACTIVE;

    PROCESS_INFORMATION pi{};
    wchar_t cmd[] = L"taskmgr.exe";

    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForInputIdle(pi.hProcess, 3000);
        if (HWND tm = GetMainWindowFromPid(pi.dwProcessId)) {
            ShowWindowAsync(tm, SW_SHOWMINNOACTIVE);
            ShowWindowAsync(tm, SW_MINIMIZE);
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

int wmain() {
    while (true) {
        if (!IsForegroundFullscreen()) {
            MinimizeTaskManagerNoActivate();
        }
        Sleep(15000); // 30 seconds
    }
    return 0;
}
