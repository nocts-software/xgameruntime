/*
 * Xbox Game runtime Library
 * WinRT CoreWindow / XAML Application Host
 *
 * Copyright 2026 Olivia Ryan
 */

#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(gdkc);

static LRESULT CALLBACK core_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 120, 215));
            DrawTextW(hdc, L"Microsoft Store (Xodus Runtime)", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

HWND create_core_window_host(void)
{
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = core_window_proc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"CoreWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        L"CoreWindow",
        L"Microsoft Store",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
        NULL, NULL, GetModuleHandleW(NULL), NULL
    );

    if (hwnd)
    {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        TRACE("Successfully created Microsoft Store CoreWindow %p!\n", hwnd);
    }
    return hwnd;
}
