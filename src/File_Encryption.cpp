// File Encryption.cpp : Defines the entry point for the application.
//

#include <Windows.h>
#include <CommCtrl.h>
#include <Richedit.h>
#include <commdlg.h>

#include<string>

#include "framework.h"
#include "File Encryption.h"

#include "core.h"

#define MAX_LOADSTRING 100
#define MAX_FILEPATH_SIZE 256

// Global Variables:
HWND hWnd;                                      //main window
HINSTANCE hInst;                                // current instance
CHAR szTitlemain[MAX_LOADSTRING];                  // The title bar text
CHAR szWindowClassmain[MAX_LOADSTRING];            // the main window class name

HBRUSH bkbrush = NULL;
COLORREF bkcolor = RGB(255, 255, 255);

LPCSTR szTitleprogress = (LPCSTR)"progress";
LPCSTR szWindowClassprogress = "title";


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance, WNDPROC  WndProcedure, LPCSTR windowname, DWORD menu);
HWND                InitInstance(HINSTANCE, int, RECT window_rectangle, LPCSTR windowname, LPCSTR windowtitle, HWND hwnd, DWORD ch);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK ProgressWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Info(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
bool encrypt_task = false, decrypt_task = false;

std::string get_filename(HWND hwnd);

HFONT create_font(HWND hWnd);
void create_progress_bar(HWND hwnd, int end);
void start_job();

std::string pswdsz;
int initial_pswdsz = 0;

bool radio_state = false;
bool no_typing = true;
bool done = false;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    window_rectangle.left = 26;
    window_rectangle.top = 26;
    window_rectangle.right = 500;
    window_rectangle.bottom = 200;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitlemain, MAX_LOADSTRING);
    LoadString(hInstance, IDC_FILEENCRYPTION, szWindowClassmain, MAX_LOADSTRING);

    MyRegisterClass(hInstance, WndProc, szWindowClassmain, IDC_FILEENCRYPTION);


    // Perform application initialization:
    hWnd = InitInstance(hInstance, nCmdShow, window_rectangle, (LPCSTR)szWindowClassmain, (LPCSTR)szTitlemain, nullptr,WS_OVERLAPPEDWINDOW);
    if (!hWnd)
          return FALSE;
    else
    {
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }
    

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FILEENCRYPTION));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance, WNDPROC  WndProcedure, LPCSTR windowname, DWORD menu)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProcedure;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILEENCRYPTION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCSTR)MAKEINTRESOURCEW(menu);
    wcex.lpszClassName  = windowname;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow, RECT window_rectangle, LPCSTR windowname, LPCSTR windowtitle, HWND parent, DWORD ch)
{
   hInst = hInstance; // Store instance handle in our global variable

  HWND hwnd = CreateWindow(windowname, windowtitle, ch, window_rectangle.left, window_rectangle.top,
      window_rectangle.right, window_rectangle.bottom, parent, nullptr, hInstance, nullptr);

    return hwnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
    switch (message)
    {
    case WM_CREATE:
        browse_icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BROWSE));
        browse_button = CreateWindow(WC_BUTTON, NULL, WS_VISIBLE | WS_CHILD | BS_ICON , browse_rectangle.left, browse_rectangle.top, browse_rectangle.right, browse_rectangle.bottom, hWnd, (HMENU)FILEPATH, nullptr, nullptr);
        SendMessage(browse_button, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browse_icon);
        browse_caption = CreateWindowA(WC_STATICA, "Browse", WS_CHILD | WS_VISIBLE, browse_caption_rectangle.left, browse_caption_rectangle.top, browse_caption_rectangle.right, browse_caption_rectangle.bottom, hWnd, nullptr, nullptr, nullptr);

        filepath_field = CreateWindowA(WC_EDITA, "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER, filepath_field_rectangle.left, filepath_field_rectangle.top, filepath_field_rectangle.right, filepath_field_rectangle.bottom, hWnd, nullptr, nullptr, nullptr);
        file_caption = CreateWindowA(WC_STATICA, "File:", WS_CHILD | WS_VISIBLE, file_caption_rectangle.left, file_caption_rectangle.top, file_caption_rectangle.right, file_caption_rectangle.bottom, hWnd, nullptr, nullptr, nullptr);

        controls_font = create_font(hWnd);

        SendMessageA(browse_caption, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));

        SendMessageA(filepath_field, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(file_caption, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(filepath_field, WM_SETTEXT, (WPARAM)nullptr, (LPARAM)(LPCSTR)"file path appears here");
               
        encryption_key_field = CreateWindowA(WC_EDITA, "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER, encryption_key_field_rectangle.left, encryption_key_field_rectangle.top, encryption_key_field_rectangle.right, encryption_key_field_rectangle.bottom, hWnd, (HMENU)ID_PASSWORD, nullptr, nullptr);
        encryption_key_caption = CreateWindowA(WC_STATICA, "Password:", WS_CHILD | WS_VISIBLE, encryption_key_caption_rectangle.left, encryption_key_caption_rectangle.top, encryption_key_caption_rectangle.right, encryption_key_caption_rectangle.bottom, hWnd, nullptr, nullptr, nullptr);
        encryption_key_pswdsz = CreateWindowA(WC_STATICA, "30", WS_CHILD | WS_VISIBLE | WS_BORDER, browse_rectangle.left, browse_rectangle.top, browse_rectangle.right, browse_rectangle.bottom, hWnd, nullptr, nullptr, nullptr);
        encryption_key_on_off = CreateWindowA(WC_BUTTONA, NULL, WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, encryption_key_field_rectangle.left + 30, encryption_key_field_rectangle.top - 20, 15, browse_rectangle.bottom - 5, hWnd, (HMENU)ID_RADIO, nullptr, nullptr);
        show_password = CreateWindowA(WC_STATICA, "Show Password", WS_CHILD | WS_VISIBLE, encryption_key_field_rectangle.left + 50, encryption_key_field_rectangle.top - 20, 90, browse_rectangle.bottom - 5, hWnd, nullptr, nullptr, nullptr);


        SendMessageA(encryption_key_field, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(encryption_key_caption, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(encryption_key_pswdsz, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(show_password, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));

        SendMessageA(encryption_key_field, WM_SETTEXT, (WPARAM)nullptr, (LPARAM)(LPCSTR)"must be 16 characters/symbols");
        
        encrypt_button = CreateWindowA(WC_BUTTONA,"Encrypt", WS_VISIBLE | WS_CHILD | WS_BORDER, encrypt_button_rectangle.left, encrypt_button_rectangle.top, encrypt_button_rectangle.right, encrypt_button_rectangle.bottom, hWnd, (HMENU)ENCRYPT, nullptr, nullptr);
        decrypt_button = CreateWindowA(WC_BUTTONA,"Decrypt", WS_VISIBLE | WS_CHILD | WS_BORDER, decrypt_button_rectangle.left, decrypt_button_rectangle.top, decrypt_button_rectangle.right, decrypt_button_rectangle.bottom, hWnd, (HMENU)DECRYPT, nullptr, nullptr);

        SendMessageA(encrypt_button, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        SendMessageA(decrypt_button, WM_SETFONT, (WPARAM)controls_font, MAKELPARAM(TRUE, 0));
        
        break;
    case WM_COMMAND:
        {  
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break; 
            case ID_PASSWORD:
                password_length = GetWindowTextLengthA(encryption_key_field);

                if ((password_length < initial_pswdsz) && (no_typing == true))
                {
                    SendMessageA(encryption_key_field, EM_SETPASSWORDCHAR, MAKEWPARAM('•', TRUE), MAKELPARAM(TRUE, 0));
                    no_typing = false;
                }
                if (password_length != initial_pswdsz)
                {
                    initial_pswdsz = password_length;
                    pswdsz = std::to_string(initial_pswdsz);
                    SendMessageA(encryption_key_pswdsz, WM_SETTEXT, (WPARAM)nullptr, (LPARAM)(LPCSTR)pswdsz.c_str());
                }
                
               
                break;
            case ID_RADIO:
                if (radio_state == false)
                {
                    SendMessageA(encryption_key_on_off, BM_SETCHECK, 1, 0);
                    SendMessageA(encryption_key_field, EM_SETPASSWORDCHAR, (WPARAM)0, MAKELPARAM(TRUE, 0));
                    SendMessageA(encryption_key_field, WM_SETFOCUS, (WPARAM)0, MAKELPARAM(TRUE, 0));
                    SendMessageA(encryption_key_field, WM_KILLFOCUS, (WPARAM)0, MAKELPARAM(TRUE, 0));
                    radio_state = true;
                }
                else
                {
                    SendMessageA(encryption_key_on_off, BM_SETCHECK, 0, 0);
                    SendMessageA(encryption_key_field, EM_SETPASSWORDCHAR, MAKEWPARAM('•', TRUE), MAKELPARAM(TRUE, 0));
                    SendMessageA(encryption_key_field, WM_SETFOCUS, (WPARAM)0, MAKELPARAM(TRUE, 0));
                    SendMessageA(encryption_key_field, WM_KILLFOCUS, (WPARAM)0, MAKELPARAM(TRUE, 0));
                    radio_state = false;
                }
                  
                break;
            case FILEPATH:
                filepath = get_filename(hWnd);

                filepath_valid = verify_filepath((const char*)filepath.c_str());

                if(filepath_valid)
                    SendMessageA(filepath_field, WM_SETTEXT, (WPARAM)nullptr, (LPARAM)(LPCSTR)filepath.c_str());
                else
                    SendMessageA(filepath_field, WM_SETTEXT, (WPARAM)nullptr, (LPARAM)(LPCSTR)invalid_filepath);
                break;
            case ENCRYPT:
                //encryption
            {
                filepath_length = GetWindowTextLengthA(filepath_field);
                GetWindowTextA(filepath_field, path, filepath_length + 1);
                filepath = path;

                filepath_valid = verify_filepath((const char*)filepath.c_str());

                password_length = GetWindowTextLengthA(encryption_key_field);
                GetWindowTextA(encryption_key_field, password, password_length + 1);
                passwordstr = password;

                if (filepath_valid == true && password_length == 16)
                {
                    encrypt_task = true;
                    start_job();
                    if (done == true)
                    {
                        MessageBox(hWnd, "Done!", "Cipher", MB_ICONINFORMATION | MB_SYSTEMMODAL);
                        done = false;
                    }
                }
                else
                    MessageBox(hWnd, "You entered invalid details", "Cipher", MB_ICONERROR);
                break;
            }
                
            case DECRYPT:
                //decrypt
            {
                filepath_length = GetWindowTextLengthA(filepath_field);
                GetWindowTextA(filepath_field, path, filepath_length + 1);
                filepath = path;

                filepath_valid = verify_filepath((const char*)filepath.c_str());

                password_length = GetWindowTextLengthA(encryption_key_field);
                GetWindowTextA(encryption_key_field, password, password_length + 1);
                passwordstr = password;

                if (filepath_valid == true && password_length == 16)
                {
                    decrypt_task = true;
                    start_job();
                    if (done == true)
                    {
                        MessageBox(hWnd, "Done!", "Cipher", MB_ICONINFORMATION | MB_SYSTEMMODAL);
                        done = false;
                    }
                    
                }
                else
                    MessageBox(hWnd, "You entered invalid details", "Cipher", MB_ICONERROR);
                break;
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    
    case WM_SIZE:        
        resize_callback(hWnd);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(bkbrush));

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

LRESULT CALLBACK ProgressWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hwndprogress = CreateWindowEx(0L, PROGRESS_CLASS, NULL, WS_CHILD | PBS_SMOOTH, 15, 15, 300, 20, hwnd, nullptr, nullptr, nullptr);
        create_progress_bar(hwndprogress, end);
        break;
    case WM_COMMAND:
        break;
    case WM_SETFOCUS:
         break;
    case WM_SIZE:
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(bkbrush));

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
      DestroyWindow(hwnd);
      break;
    default:
    {
       return DefWindowProc(hwnd, message, wParam, lParam);
    }
    }

    return 0;
}

// Message handler for about box.
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

INT_PTR CALLBACK Info(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (Msg)
    {
    case WM_INITDIALOG:
    {
        int left = (window_rectangle.right - window_rectangle.left) / 2 + window_rectangle.left;
        int top = (window_rectangle.bottom - window_rectangle.top) / 2 + window_rectangle.top;

        GetWindowRect(hWndDlg, &info_dialog_rectangle);
        SetWindowPos(hWndDlg, NULL, left, top, 470, 170, SWP_SHOWWINDOW);

    }
         return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK )
        {
            done = true;
            EndDialog(hWndDlg, LOWORD(wParam));
            if (encrypt_task == true)
            {
                encrypt(filepath.c_str(), passwordstr.c_str());
                encrypt_task = false;
            }
            else if (decrypt_task == true)
            {
                decrypt(filepath.c_str(), passwordstr.c_str());
                decrypt_task = false;
            }
               

            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hWndDlg, LOWORD(wParam));
            SendMessage(hWndprogress, WM_CLOSE, NULL, 0);
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hWndDlg, LOWORD(wParam));
        SendMessage(hWndprogress, WM_CLOSE, NULL, 0);
        return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}

std::string get_filename(HWND hwnd)
{
    OPENFILENAMEA ofn; //common dialog box structure
    CHAR szFile[MAX_FILEPATH_SIZE];  // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = (LPSTR)"All Files\0*.*\0Source Files\0*.CPP*\0Text Files\0*.TXT*\0Image files\0*.JPG*\0.PNG\0Bitmap files\0*.BMP*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    // Display the Open dialog box.

    if (GetOpenFileNameA(&ofn) == TRUE )
        return szFile;
    else
        return "";
}

HFONT create_font(HWND hWnd)
{
    const TCHAR* fontName = _T("Calibri");
    const long nFontSize = 10;

    HDC hdc = GetDC(hWnd);

    LOGFONT logFont = { 0 };
    logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    logFont.lfWeight = FW_BOLD;
    _tcscpy_s(logFont.lfFaceName, fontName);

    HFONT s_hFont;
    s_hFont = CreateFontIndirect(&logFont);

    ReleaseDC(hWnd, hdc);

    return s_hFont;
}

void create_progress_bar(HWND hwndprogress, int end)
{    
    ShowWindow(hwndprogress, SW_SHOW);

    SendMessage(hwndprogress, PBM_SETRANGE, 0, MAKELPARAM(TRUE, end));
    SendMessage(hwndprogress, PBM_SETSTEP, (WPARAM)1, 0);
    SendMessage(hwndprogress, PBM_SETBARCOLOR, 0, RGB(38, 171, 35));
}

void start_job()
{
    setup_progressbar_data(filepath.c_str());
    MyRegisterClass(hInst, ProgressWndProc, szWindowClassprogress, NULL);

    GetWindowRect(hWnd, &window_rectangle);
    progress_windowrectangle.left = (window_rectangle.right - window_rectangle.left)/2 - progress_windowrectangle.right/2;
    progress_windowrectangle.top = (window_rectangle.bottom - window_rectangle.top) / 2 - progress_windowrectangle.bottom/2;
    progress_windowrectangle.right = 350;
    progress_windowrectangle.bottom = 100;

    // Perform application initialization:
    hWndprogress = InitInstance(hInst, SW_SHOW, progress_windowrectangle, szWindowClassprogress, szTitleprogress, hWnd,
       WS_OVERLAPPEDWINDOW);
    
    DialogBox(hInst, MAKEINTRESOURCE(IDD_Info), hWnd, Info);
}