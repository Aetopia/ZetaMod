// Source code for Window Display Mode Tool.
#include <windows.h>
#include <libgen.h>
#include <psapi.h>
#include <shellscalingapi.h>

// Prototypes

// Structure that contains information on the hooked process' window.
struct WINDOW;

// Check if the current foreground window is the hooked process' window.
BOOL IsProcWndForeground(struct WINDOW *wnd);

// Check for a specific foreground window via its PID and once hooked, set the display mode.
void CheckForegroundWndPID(struct WINDOW *wnd);

// Check the hooked process is alive.
void IsWndProcAlive(struct WINDOW *wnd);

// Apply the desired resolution when the hooked process is in the foreground.
void SetForegroundWndDM(struct WINDOW *wnd);

// Reset to the native resolution when the hooked process is not in the foreground.
void ResetForegroundWndDM(struct WINDOW *wnd);

struct WINDOW
{
    HANDLE hproc;  // HANDLE to the hooked process.
    DEVMODE *dm;   // Display mode to be applied when the hooked process' window is in the foreground
    BOOL reset;    // Reset the display mode back to default.
    DWORD process; // PID of the hooked process.
    char *monitor; // Name of the monitor, the window is present on.
    DWORD ec, pid; // Reserved exit code and PID variables.
    HWND hwnd;     // Reversed HWND variable.
};

BOOL IsProcWndForeground(struct WINDOW *wnd)
{
    // Sometimes, some programs might have multiple HWNDs and WDMT might not switch display resolution due to fetching the incorrect HWND.
    wnd->hwnd = GetForegroundWindow();
    GetWindowThreadProcessId(wnd->hwnd, &wnd->pid);
    if (wnd->process == wnd->pid)
    {
        return FALSE;
    };
    return TRUE;
}

void CheckForegroundWndPID(struct WINDOW *wnd)
{
    wnd->hproc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, wnd->process);
    if (!wnd->hproc)
    {
        CloseHandle(wnd->hproc);
        MessageBox(0,
                   "Invaild PID!",
                   "Window Display Mode Tool",
                   MB_ICONEXCLAMATION);
        exit(1);
    }
    do
    {
        Sleep(1);
    } while (!IsProcWndForeground());
}

void IsProcAlive(struct WINDOW *wnd)
{
    GetExitCodeProcess(wnd->hproc, &wnd->ec);
    if (wnd->ec != STILL_ACTIVE)
    {
        CloseHandle(wnd->hproc);
        for (;;)
        {
            if (wnd->reset)
            {
                if (ChangeDisplaySettingsEx(wnd->monitor,
                                            0,
                                            NULL,
                                            CDS_FULLSCREEN,
                                            NULL) == DISP_CHANGE_SUCCESSFUL)
                {
                    ChangeDisplaySettingsEx(wnd->monitor, 0, NULL, 0, NULL);
                    exit(0);
                };
            }
            else
            {
                exit(0);
            };
        };
    };
    Sleep(1);
}

void SetForegroundWndDM(struct WINDOW *wnd)
{
    wnd->reset = FALSE;
    do
    {
        IsProcAlive(wnd);
    } while (IsProcWndForeground(wnd));
    if (ChangeDisplaySettingsEx(wnd->monitor,
                                wnd->dm,
                                NULL,
                                CDS_FULLSCREEN,
                                NULL) == DISP_CHANGE_SUCCESSFUL)
    {
        ResetForegroundWndDM(wnd);
    };
}

void ResetForegroundWndDM(struct WINDOW *wnd)
{
    wnd->reset = TRUE;
    do
    {
        IsProcAlive(wnd);
    } while (!IsProcWndForeground(wnd));
    if (ChangeDisplaySettingsEx(wnd->monitor, 0, NULL, CDS_FULLSCREEN, NULL) == DISP_CHANGE_SUCCESSFUL)
    {
        ChangeDisplaySettingsEx(wnd->monitor, 0, NULL, 0, NULL);
        SetForegroundWndDM(wnd);
    };
}

int main(int argc, char *argv[])
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    struct WINDOW wnd;
    DEVMODE dm, cdm;
    MONITORINFOEX mi;
    HMONITOR hmon;
    UINT dpiX, dpiY, dpiC = GetDpiForSystem();
    mi.cbSize = sizeof(mi);
    dm.dmSize = sizeof(dm);
    cdm.dmSize = sizeof(cdm);

    if (argc != 4)
    {
        MessageBox(0,
                   "WDMT.exe <PID> <Width> <Height>",
                   "Window Display Mode Tool",
                   MB_ICONINFORMATION);
        return 0;
    };

    // Setup the Display Mode.
    // Display resolution to be used.
    dm.dmPelsWidth = atoi(argv[2]);
    dm.dmPelsHeight = atoi(argv[3]);
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    wnd.dm = &dm;

    // Check if specified resolution is valid or not.
    if (ChangeDisplaySettings(wnd.dm, CDS_TEST) != DISP_CHANGE_SUCCESSFUL ||
        (dm.dmPelsWidth || dm.dmPelsHeight) == 0)
    {
        MessageBox(0,
                   "Invaild Resolution!",
                   "Window Display Mode Tool",
                   MB_ICONEXCLAMATION);
        return 1;
    }

    // Check if the <PID/Process> argument contains only integers or not.
    if (strspn(argv[1], "0123456789") == strlen(argv[1]))
    {
        wnd.process = atoi(argv[1]);
        CheckForegroundWndPID(&wnd);
    }
    else
    {
        MessageBox(0,
                   "Invaild PID!",
                   "Window Display Mode Tool",
                   MB_ICONEXCLAMATION);
        return 1;
    };

    // Source: https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    // Restore the window if its maximized.
    if (IsZoomed(wnd.hwnd))
    {
        ShowWindow(wnd.hwnd, SW_RESTORE);
    };

    // Get the monitor, the window is present on.
    hmon = MonitorFromWindow(wnd.hwnd, MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo(hmon, (MONITORINFO *)&mi);
    // Name of the monitor, the window is present on.
    wnd.monitor = mi.szDevice;
    // Get the current resolution of the monitor the window is on.
    EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &cdm);

    // Set the window style to borderless.
    SetWindowLongPtr(wnd.hwnd, GWL_STYLE,
                     GetWindowLongPtr(wnd.hwnd, GWL_STYLE) & ~(WS_OVERLAPPEDWINDOW));
    SetWindowLongPtr(wnd.hwnd, GWL_EXSTYLE,
                     GetWindowLongPtr(wnd.hwnd, GWL_EXSTYLE) & ~(WS_EX_OVERLAPPEDWINDOW));

    // If the specified resolution is not the same as the display native resolution then execute ResetForegroundWndDM(struct WINDOW *wnd).
    // This code block additionally sizes the window based on the DPI scaling set by the desired display resolution.
    if (dm.dmPelsHeight != cdm.dmPelsHeight && dm.dmPelsWidth != cdm.dmPelsWidth)
    {
        ChangeDisplaySettingsEx(wnd.monitor, wnd.dm, NULL, CDS_FULLSCREEN, NULL);
        GetDpiForMonitor(hmon, 0, &dpiX, &dpiY);
        SetWindowPos(wnd.hwnd,
                     HWND_TOP,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     dm.dmPelsWidth * (float)dpiC / dpiX,
                     dm.dmPelsHeight * (float)dpiC / dpiY,
                     SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
        ResetForegroundWndDM(&wnd);
    }
    SetWindowPos(wnd.hwnd,
                 HWND_TOP,
                 mi.rcMonitor.left,
                 mi.rcMonitor.top,
                 dm.dmPelsWidth,
                 dm.dmPelsHeight,
                 SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
    return 0;
}