// mfc_demo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "mfc_demo.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

static Mat g_matImage;
static std::vector<Rect> g_faces;
static std::wstring g_currentFilePath;
static HWND g_hWndMain = nullptr;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void OpenImageFile(HWND hWnd);
void DetectFaces(Mat& image);
void DrawImageWithFaces(HWND hWnd, HDC hdc);
std::wstring GetOpenFileNameDialog(HWND hWnd);
std::string WStringToString(const std::wstring& wstr);
std::wstring StringToWString(const std::string& str);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MFCDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MFCDEMO));

    MSG msg;

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



ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MFCDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MFCDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWndMain = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_OPEN:
                OpenImageFile(hWnd);
                break;
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
            DrawImageWithFaces(hWnd, hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        g_matImage.release();
        g_faces.clear();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

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

std::wstring GetOpenFileNameDialog(HWND hWnd)
{
    OPENFILENAME ofn;
    WCHAR szFile[MAX_PATH] = { 0 };
    WCHAR szFilter[] = L"图片文件\0*.jpg;*.jpeg;*.png;*.bmp\0JPEG文件\0*.jpg;*.jpeg\0PNG文件\0*.png\0所有文件\0*.*\0";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        return std::wstring(szFile);
    }
    return L"";
}

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
    return str;
}

std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
    return wstr;
}

void OpenImageFile(HWND hWnd)
{
    std::wstring filePath = GetOpenFileNameDialog(hWnd);
    if (filePath.empty()) return;

    g_currentFilePath = filePath;
    g_faces.clear();

    std::string strPath = WStringToString(filePath);
    g_matImage = imread(strPath);

    if (g_matImage.empty())
    {
        MessageBox(hWnd, L"无法加载图片，请选择有效的JPG或PNG文件。", L"错误", MB_ICONERROR);
        return;
    }

    DetectFaces(g_matImage);
    InvalidateRect(hWnd, NULL, TRUE);
}

void DetectFaces(Mat& image)
{
    g_faces.clear();

    if (image.empty()) return;

    CascadeClassifier faceCascade;
    std::vector<std::wstring> possiblePaths;
    
    possiblePaths.push_back(L"F:\\VC\\SDK\\opencv411\\build\\etc\\haarcascades\\haarcascade_frontalface_default.xml");
    possiblePaths.push_back(L"F:\\VC\\SDK\\opencv411\\etc\\haarcascades\\haarcascade_frontalface_default.xml");
    
    WCHAR szExePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, szExePath, MAX_PATH);
    std::wstring exeDir = szExePath;
    size_t pos = exeDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        exeDir = exeDir.substr(0, pos + 1);
    }
    possiblePaths.push_back(exeDir + L"haarcascade_frontalface_default.xml");
    possiblePaths.push_back(exeDir + L"haarcascades\\haarcascade_frontalface_default.xml");

    bool loaded = false;
    for (const auto& path : possiblePaths)
    {
        std::string strPath = WStringToString(path);
        if (faceCascade.load(strPath))
        {
            loaded = true;
            break;
        }
    }

    if (!loaded)
    {
        MessageBox(g_hWndMain, 
            L"无法加载人脸检测分类器文件。\n"
            L"请确保 haarcascade_frontalface_default.xml 文件在以下位置之一：\n"
            L"1. F:\\VC\\SDK\\opencv411\\build\\etc\\haarcascades\\\n"
            L"2. F:\\VC\\SDK\\opencv411\\etc\\haarcascades\\\n"
            L"3. 程序运行目录",
            L"警告", MB_ICONWARNING);
        return;
    }

    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);

    faceCascade.detectMultiScale(gray, g_faces, 1.1, 3, 0, Size(30, 30));
}

void DrawImageWithFaces(HWND hWnd, HDC hdc)
{
    RECT rectClient;
    GetClientRect(hWnd, &rectClient);
    int clientWidth = rectClient.right - rectClient.left;
    int clientHeight = rectClient.bottom - rectClient.top;

    if (g_matImage.empty())
    {
        FillRect(hdc, &rectClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
        return;
    }

    int imgWidth = g_matImage.cols;
    int imgHeight = g_matImage.rows;

    double scaleX = (double)clientWidth / imgWidth;
    double scaleY = (double)clientHeight / imgHeight;
    double scale = min(scaleX, scaleY);

    int drawWidth = (int)(imgWidth * scale);
    int drawHeight = (int)(imgHeight * scale);
    int offsetX = (clientWidth - drawWidth) / 2;
    int offsetY = (clientHeight - drawHeight) / 2;

    Mat matToDraw;
    if (scale != 1.0)
    {
        resize(g_matImage, matToDraw, Size(drawWidth, drawHeight), 0, 0, INTER_AREA);
    }
    else
    {
        matToDraw = g_matImage.clone();
    }

    for (const auto& face : g_faces)
    {
        Rect scaledFace(
            (int)(face.x * scale),
            (int)(face.y * scale),
            (int)(face.width * scale),
            (int)(face.height * scale)
        );
        rectangle(matToDraw, scaledFace, Scalar(0, 0, 255), 2);
    }

    Mat matBGR;
    if (matToDraw.channels() == 3)
    {
        cvtColor(matToDraw, matBGR, COLOR_BGR2BGRA);
    }
    else if (matToDraw.channels() == 1)
    {
        cvtColor(matToDraw, matBGR, COLOR_GRAY2BGRA);
    }
    else
    {
        matBGR = matToDraw.clone();
    }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = matBGR.cols;
    bmi.bmiHeader.biHeight = -matBGR.rows;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    FillRect(hdc, &rectClient, (HBRUSH)GetStockObject(COLOR_WINDOW + 1));

    StretchDIBits(hdc,
        offsetX, offsetY,
        drawWidth, drawHeight,
        0, 0,
        matBGR.cols, matBGR.rows,
        matBGR.data,
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY);

    if (!g_faces.empty())
    {
        WCHAR szText[100];
        swprintf_s(szText, L"检测到 %d 个人脸", (int)g_faces.size());
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, 10, 10, szText, (int)wcslen(szText));
    }
}
