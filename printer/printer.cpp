// printer.cpp : 定義應用程式的進入點。
//

#include "framework.h"
#include "printer.h"
#include <vector> 
#include <string>
#include <commdlg.h> // 引入 Windows 通用對話方塊函式庫（包含調色盤、存檔對話框）

#define MAX_LOADSTRING 100

// 定義選單與功能的 ID 識別碼
#define IDM_CHOOSE_COLOR 60001
#define IDM_SAVE_IMAGE   60002 
#define IDM_UNDO         60003 
#define IDM_USE_PEN      60004 // 新增：使用畫筆
#define IDM_USE_ERASER   60005 // 新增：使用橡皮擦

// ==================== 資料結構定義 ====================
struct Point {
    int x;
    int y;
};

struct Stroke {
    std::vector<Point> points;
    COLORREF color;
    int thickness;
};

// 全域變數:
HINSTANCE hInst;                                // 目前執行個體
WCHAR szTitle[MAX_LOADSTRING];                  // 標題列文字
WCHAR szWindowClass[MAX_LOADSTRING];            // 主視窗類別名稱

// 儲存歷史筆劃的全域變數
std::vector<Stroke> g_History;

// 這個程式碼模組所包含之函式的向前宣告:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                DrawHistory(HDC hdc);

// 原生 Win32 GDI 存檔功能（只使用系統內建 API）
BOOL                SaveBitmapNativeGDI(HBITMAP hBitmap, const wchar_t* filename);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 將全域字串初始化
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PRINTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 執行應用程式初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRINTER));

    MSG msg;

    // 主訊息迴圈:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//  函式: MyRegisterClass()
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRINTER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PRINTER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//   函式: InitInstance(HINSTANCE, int)
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1024, 768, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // ==================== 動態修改上方功能表 (Menu) ====================
    HMENU hMenu = GetMenu(hWnd);
    if (hMenu) {
        // 1. 處理原本的「檔案」選單：直接在第一個子選單的最下方新增「儲存圖片」
        HMENU hFileSubMenu = GetSubMenu(hMenu, 0);
        if (hFileSubMenu) {
            AppendMenuW(hFileSubMenu, MF_SEPARATOR, 0, NULL); // 分隔線
            AppendMenuW(hFileSubMenu, MF_STRING, IDM_SAVE_IMAGE, L"儲存圖片(&S)...\tCtrl+S");
        }

        // 2. 處理「色彩」選單：建立色彩管理子選單
        HMENU hColorSubMenu = CreatePopupMenu();
        AppendMenuW(hColorSubMenu, MF_STRING, IDM_CHOOSE_COLOR, L"選擇顏色(&C)...\tC");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hColorSubMenu, L"色彩(&C)");

        // 3. 處理「動作」選單：加入復原、畫筆、橡皮擦
        HMENU hActionSubMenu = CreatePopupMenu();
        AppendMenuW(hActionSubMenu, MF_STRING, IDM_USE_PEN, L"使用畫筆(&P)\tCtrl+P");
        AppendMenuW(hActionSubMenu, MF_STRING, IDM_USE_ERASER, L"使用橡皮擦(&E)\tCtrl+E");
        AppendMenuW(hActionSubMenu, MF_SEPARATOR, 0, NULL); // 分隔線
        AppendMenuW(hActionSubMenu, MF_STRING, IDM_UNDO, L"復原上一筆(&U)\tCtrl+Z");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hActionSubMenu, L"動作(&A)");

        // 通知視窗重新整理頂部功能表
        DrawMenuBar(hWnd);
    }
    // ========================================================================

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 負責繪製所有歷史筆劃的函式
void DrawHistory(HDC hdc) {
    for (const auto& stroke : g_History) {
        if (stroke.points.size() < 2) continue;

        HPEN hPen = CreatePen(PS_SOLID, stroke.thickness, stroke.color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        MoveToEx(hdc, stroke.points[0].x, stroke.points[0].y, NULL);
        for (size_t i = 1; i < stroke.points.size(); ++i) {
            LineTo(hdc, stroke.points[i].x, stroke.points[i].y);
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// 完全不使用外部函式庫，純靠 Win32 檔案 API 組裝點陣圖檔案結構
BOOL SaveBitmapNativeGDI(HBITMAP hBitmap, const wchar_t* filename) {
    HDC hDC = GetDC(NULL);
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    int rowWidth = ((width * 24 + 31) / 32) * 4;
    DWORD dataSize = rowWidth * height;

    std::vector<BYTE> buffer(dataSize);
    GetDIBits(hDC, hBitmap, 0, height, buffer.data(), &bi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hDC);

    BITMAPFILEHEADER bfh;
    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    DWORD written;
    WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &written, NULL);
    WriteFile(hFile, &bi.bmiHeader, sizeof(BITMAPINFOHEADER), &written, NULL);
    WriteFile(hFile, buffer.data(), dataSize, &written, NULL);

    CloseHandle(hFile);
    return TRUE;
}

//  函式: WndProc(HWND, UINT, WPARAM, LPARAM)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL isDrawing = FALSE;
    static BOOL isEraserMode = FALSE;          // 新增：目前是否為橡皮擦模式
    static COLORREF currentColor = RGB(0, 0, 0);
    static int currentThickness = 3;
    static int eraserThickness = 20;           // 新增：橡皮擦粗細度較粗

    static COLORREF acrCustClr[16] = {
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255),
        RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255)
    };

    switch (message)
    {
    case WM_LBUTTONDOWN: {
        isDrawing = TRUE;
        Stroke newStroke;

        // 根據目前模式決定畫筆顏色與粗細
        if (isEraserMode) {
            newStroke.color = RGB(255, 255, 255); // 橡皮擦就是白色
            newStroke.thickness = eraserThickness;
        }
        else {
            newStroke.color = currentColor;
            newStroke.thickness = currentThickness;
        }

        Point p = { LOWORD(lParam), HIWORD(lParam) };
        newStroke.points.push_back(p);
        g_History.push_back(newStroke);
        break;
    }

    case WM_MOUSEMOVE: {
        if (isDrawing && !g_History.empty()) {
            Point p = { LOWORD(lParam), HIWORD(lParam) };
            g_History.back().points.push_back(p);

            HDC hdc = GetDC(hWnd);

            // 決定畫布畫線時即時呈現的顏色與粗細
            COLORREF drawColor = isEraserMode ? RGB(255, 255, 255) : currentColor;
            int drawThickness = isEraserMode ? eraserThickness : currentThickness;

            HPEN hPen = CreatePen(PS_SOLID, drawThickness, drawColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            size_t sz = g_History.back().points.size();
            Point lastP = g_History.back().points[sz - 2];

            MoveToEx(hdc, lastP.x, lastP.y, NULL);
            LineTo(hdc, p.x, p.y);

            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
            ReleaseDC(hWnd, hdc);
        }
        break;
    }

    case WM_LBUTTONUP:
        isDrawing = FALSE;
        break;

    case WM_KEYDOWN: {
        // 鍵盤 Ctrl + Z: 復原
        if (wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hWnd, WM_COMMAND, IDM_UNDO, 0);
        }
        // 鍵盤 Ctrl + S: 存檔
        else if (wParam == 'S' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hWnd, WM_COMMAND, IDM_SAVE_IMAGE, 0);
        }
        // 鍵盤 Ctrl + E: 切換橡皮擦
        else if (wParam == 'E' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hWnd, WM_COMMAND, IDM_USE_ERASER, 0);
        }
        // 鍵盤 Ctrl + P: 切換回畫筆
        else if (wParam == 'P' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hWnd, WM_COMMAND, IDM_USE_PEN, 0);
        }
        // 按 C 開啟調色盤
        else if (wParam == 'C') {
            SendMessage(hWnd, WM_COMMAND, IDM_CHOOSE_COLOR, 0);
        }
        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_USE_PEN: { // 切換為普通普通畫筆模式
            isEraserMode = FALSE;
            break;
        }
        case IDM_USE_ERASER: { // 切換為橡皮擦模式
            isEraserMode = TRUE;
            break;
        }
        case IDM_UNDO: {
            if (!g_History.empty()) {
                g_History.pop_back();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }
        case IDM_CHOOSE_COLOR: {
            CHOOSECOLOR cc;
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hWnd;
            cc.lpCustColors = (LPDWORD)acrCustClr;
            cc.rgbResult = currentColor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColor(&cc) == TRUE) {
                currentColor = cc.rgbResult;
                isEraserMode = FALSE; // 只要重新選擇顏色，就自動幫使用者切換回畫筆模式
            }
            break;
        }
        case IDM_SAVE_IMAGE: {
            OPENFILENAMEW ofn;
            wchar_t szFileName[MAX_PATH] = L"untitled.bmp";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"BMP 點陣圖 (*.bmp)\0*.bmp\0所有檔案 (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | FILE_ATTRIBUTE_NORMAL | OFN_OVERWRITEPROMPT;
            ofn.lpstrDefExt = L"bmp";

            if (GetSaveFileNameW(&ofn) == TRUE) {
                HDC hdc = GetDC(hWnd);
                RECT rect;
                GetClientRect(hWnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;

                HDC memDC = CreateCompatibleDC(hdc);
                HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
                HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

                HBRUSH hBackground = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(memDC, &rect, hBackground);
                DeleteObject(hBackground);

                DrawHistory(memDC);

                SelectObject(memDC, oldBitmap);

                if (SaveBitmapNativeGDI(memBitmap, szFileName)) {
                    MessageBoxW(hWnd, L"圖片儲存成功(BMP)！", L"成功", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    MessageBoxW(hWnd, L"圖片儲存失敗！", L"錯誤", MB_OK | MB_ICONERROR);
                }

                DeleteObject(memBitmap);
                DeleteDC(memDC);
                ReleaseDC(hWnd, hdc);
            }
            break;
        }
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH hBackground = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &rect, hBackground);
        DeleteObject(hBackground);

        DrawHistory(memDC);

        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// [關於] 方塊的訊息處理常式。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}