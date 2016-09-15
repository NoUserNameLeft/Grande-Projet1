// BTtrenlop.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BTtrenlop.h"
#include <Shlobj.h>
#include <windows.h>
#include <Strsafe.h>
#include <tchar.h>
#include <vector>
#include <Commctrl.h>
#include <Thumbcache.h>
#include <Shobjidl.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



#define MAX_LOADSTRING 100
#define IDM_CODE_SAMPLES 123
#define BUTTON_BROWSE 13014
#define BUTTON_SEARCH 5134
#define ID_TREEVIEW 654
#define ID_BROWSE 555
#define ID_SEARCH 666



// Global Variables:
HINSTANCE						hInst;									// current instance
WCHAR							szTitle[MAX_LOADSTRING];                // The title bar text
WCHAR							szWindowClass[MAX_LOADSTRING];          // the main window class name
HWND							hWndLV;
HWND							 hwndTV;
HWND							hWndTxtB_DIR;
HWND							hWndTxtB_FILE;
HWND							hWndBtt_Browse;
HWND							hWndBtt_Search;
TCHAR							szDir[MAX_PATH], szb4Dir[MAX_PATH], szafDir[MAX_PATH], szDirsrc[MAX_PATH];
TCHAR							lpszPassword[MAX_PATH];
std::vector<WIN32_FIND_DATA>	ffd, ffdCoC;
std::vector<TCHAR*>				CopyOrCut;
BOOL							COPY;
BOOL							backed = FALSE;
INT								uCountFindData = 0;
int								EnterPressed = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND				CreateListView(HWND hwndParent);	
BOOL				InitListViewCol(HWND ListViewhWnd);
BOOL				InitListViewItem(HWND ListViewhWnd);
BOOL				ShowContent(HWND hWnd, TCHAR szDir[]);
//HBITMAP				GetThumbnail(PTSTR lpszFileName);
void				ReportError(LPCTSTR lpCaption);
void				OnSafeSubclass(HWND hWnd, SUBCLASSPROC scProc, int iControl);
void				OnSafeUnSubclass(HWND hWnd, SUBCLASSPROC scProc, int iControl);
LRESULT	CALLBACK	LVProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT	CALLBACK	BrowseProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT	CALLBACK	SearchProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
int CALLBACK		CompareListItems(LPARAM, LPARAM, LPARAM);
void				OnColumnClick(LPNMLISTVIEW pLVInfo);
LPWSTR				GetType(const WIN32_FIND_DATA &fd);
INT_PTR CALLBACK	CreateFolderProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BTTRENLOP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BTTRENLOP));

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
/*		SORT FUNCTIONS		*/
void OnColumnClick(LPNMLISTVIEW pLVInfo)
{
	static int nSortColumn = 0;
	static BOOL bSortAscending = TRUE;
	LPARAM lParamSort;

	// get new sort parameters
	if (pLVInfo->iSubItem == nSortColumn)
		bSortAscending = !bSortAscending;
	else
	{
		nSortColumn = pLVInfo->iSubItem;
		bSortAscending = TRUE;
	}

	// combine sort info into a single value we can send to our sort function
	lParamSort = 1 + nSortColumn;
	if (!bSortAscending)
		lParamSort = -lParamSort;

	// sort list
	ListView_SortItems(pLVInfo->hdr.hwndFrom, CompareListItems, lParamSort);
}

int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	BOOL bSortAscending = (lParamSort > 0);
	int nColumn = abs(lParamSort) - 1;

	return bSortAscending ? (lParam1 - lParam2) : (lParam2 - lParam1);
}


//		Tao column cho LV
BOOL InitListViewCol(HWND ListViewhWnd)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvc.iSubItem = 0;
	lvc.pszText = TEXT("Name");
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(ListViewhWnd, 0, &lvc) == -1)
	{
		return FALSE;
	}
	lvc.iSubItem = 1;
	lvc.pszText = TEXT("Type");
	lvc.cx = 150;
	if (ListView_InsertColumn(ListViewhWnd, 1, &lvc) == -1)
	{
		return FALSE;
	}
	lvc.iSubItem = 2;
	lvc.pszText = TEXT("File size");
	lvc.cx = 150;
	if (ListView_InsertColumn(ListViewhWnd, 2, &lvc) == -1)
	{
		return FALSE;
	}
	lvc.iSubItem = 3;
	lvc.pszText = TEXT("Creation time");
	lvc.cx = 150;
	if (ListView_InsertColumn(ListViewhWnd, 3, &lvc) == -1)
	{
		return FALSE;
	}
	return TRUE;

}
//		Them item cho LV
BOOL InitListViewItem(HWND ListViewhWnd)
{
	LVITEM lvi;
	TCHAR buffer[256];
	WIN32_FIND_DATA FFD;
	lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
	lvi.stateMask = 0;
	//lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.iImage = 0;

	for(int index = 0; index < ffd.size(); index++)
	{
		FFD = ffd[index];
		//MessageBox(ListViewhWnd, FFD.cFileName, FFD.cFileName, MB_OK);
		lvi.iItem = index;
		lvi.pszText = FFD.cFileName;
		lvi.iSubItem = 0;

		// Insert items into the list.
		if (ListView_InsertItem(ListViewhWnd, &lvi) == -1)
			return FALSE;

		ListView_SetItemText(ListViewhWnd, index, 1, GetType(FFD));

		LARGE_INTEGER size;
		size.LowPart = FFD.nFileSizeLow;
		size.HighPart = FFD.nFileSizeHigh;
		StringCbPrintf(buffer, 255, TEXT("%ld"), size.QuadPart);
		ListView_SetItemText(ListViewhWnd, index, 2, buffer);

		SYSTEMTIME stUTC, stLOCAL;
		FileTimeToSystemTime(&FFD.ftCreationTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLOCAL);
		StringCchPrintf(buffer, 255,
			TEXT("%02d/%02d/%d %02d:%02d"),
			stLOCAL.wDay, stLOCAL.wMonth, stLOCAL.wYear,
			stLOCAL.wHour, stLOCAL.wMinute);
		ListView_SetItemText(ListViewhWnd, index, 3, buffer);

	}
	return TRUE;
}
//		Tao LV
HWND CreateListView(HWND hwndParent)
{
	RECT rcClient;                 

	GetClientRect(hwndParent, &rcClient);
	HWND hWndListView = CreateWindowEx(NULL, WC_LISTVIEW,
		L"",
		WS_CHILD | LVS_REPORT | WS_VISIBLE,
		100, 70,
		rcClient.right - rcClient.left - 100,
		rcClient.bottom - rcClient.top - 40, 
		hwndParent,
		(HMENU)IDM_CODE_SAMPLES,
		hInst,
		NULL);
	return (hWndListView);
}
//not yet
HWND CreateATreeView(HWND hwndParent)
{
	RECT rcClient;  // dimensions of client area 
	   // handle to tree-view control 

					// Ensure that the common control DLL is loaded. 
	InitCommonControls();

	// Get the dimensions of the parent window's client area, and create 
	// the tree-view control. 
	GetClientRect(hwndParent, &rcClient);
	hwndTV = CreateWindowEx(0,
		WC_TREEVIEW,
		TEXT("Tree View"),
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
		0,
		0,
		rcClient.right,
		rcClient.bottom,
		hwndParent,
		(HMENU)ID_TREEVIEW,
		hInst,
		NULL);

	// Initialize the image list, and add items to the control. 
	// InitTreeViewImageLists and InitTreeViewItems are application- 
	// defined functions, shown later. 
	/*if (!InitTreeViewImageLists(hwndTV) || !InitTreeViewItems(hwndTV))
	{
		DestroyWindow(hwndTV);
		return FALSE;
	}*/
	return hwndTV;
}
//		Lay kieu cua file
LPWSTR GetType(const WIN32_FIND_DATA &fd)
{
	int nDotPos = StrRStrI(fd.cFileName, NULL, _T(".")) - fd.cFileName;
	int len = wcslen(fd.cFileName);
	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return _T("Folder");

	if (nDotPos < 0 || nDotPos >= len) 
		return _T("Unknown");

	TCHAR *szExtension = new TCHAR[len - nDotPos + 1];
	int i;

	for (i = nDotPos; i < len; ++i)
	{
		szExtension[i - nDotPos] = fd.cFileName[i];
	}
	szExtension[i - nDotPos] = NULL; 

	if (!StrCmpI(szExtension, _T(".htm")) || !StrCmpI(szExtension, _T(".html")))
	{
		return _T("Web page");
	}
	TCHAR pszOut[256];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 256;

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExtension, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return _T("Unknown");
	}

	if (RegQueryValueEx(hKey, NULL, NULL, &dwType, (PBYTE)pszOut, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return _T("Unknown");
	}
	RegCloseKey(hKey);

	TCHAR *pszPath = new TCHAR[1000];
	dwSize = 1000;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, pszOut, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return _T("Unknown");
	}

	if (RegQueryValueEx(hKey, NULL, NULL, &dwType, (PBYTE)pszPath, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return _T("Unknown");
	}
	RegCloseKey(hKey);

	return pszPath;
}
//		Ham cho dialog Create - dat ten cho folder moi
INT_PTR CALLBACK CreateFolderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	WORD cchPassword;

	switch (message)
	{
	case WM_INITDIALOG:
		// Set password character to a plus sign (+) 
		/*SendDlgItemMessage(hDlg,
		IDE_PASSWORDEDIT,
		EM_SETPASSWORDCHAR,H
		(WPARAM) '+',
		(LPARAM)0);*/

		// Set the default push button to "Cancel." 
		SendMessage(hDlg,
			DM_SETDEFID,
			(WPARAM)IDCANCEL,
			(LPARAM)0);

		return TRUE;

	case WM_COMMAND:
		// Set the default push button to "OK" when the user enters text. 
		if (HIWORD(wParam) == EN_CHANGE &&
			LOWORD(wParam) == IDC_EDIT1)
		{
			SendMessage(hDlg,
				DM_SETDEFID,
				(WPARAM)IDOK,
				(LPARAM)0);
		}
		switch (wParam)
		{
		case IDOK:
			// Get number of characters. 
			cchPassword = (WORD)SendDlgItemMessage(hDlg,
				IDC_EDIT1,
				EM_LINELENGTH,
				(WPARAM)0,
				(LPARAM)0);
			/*if (cchPassword >= 16)
			{
			MessageBox(hDlg,
			L"Too many characters.",
			L"Error",
			MB_OK);

			EndDialog(hDlg, TRUE);
			return FALSE;
			}
			else if (cchPassword == 0)
			{
			MessageBox(hDlg,
			L"No characters entered.",
			L"Error",
			MB_OK);

			EndDialog(hDlg, TRUE);
			return FALSE;
			}*/

			// Put the number of characters into first word of buffer. 
			*((LPWORD)lpszPassword) = cchPassword;

			// Get the characters. 
			SendDlgItemMessage(hDlg,
				IDC_EDIT1,
				EM_GETLINE,
				(WPARAM)0,       // line 0 
				(LPARAM)lpszPassword);

			// Null-terminate the string. 
			lpszPassword[cchPassword] = 0;
			//MessageBox(hDlg, lpszPassword, lpszPassword, MB_OK);
			//TextOut(memdc, );
			//MessageBox(hDlg,
			//	lpszPassword,
			//	L"Did it work?",
			//	MB_OK);

			// Call a local password-parsing function. 
			//ParsePassword(lpszPassword);

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		return 0;
	}
	return FALSE;

	UNREFERENCED_PARAMETER(lParam);
}


//IShellItem* GetShellItem(PTSTR lpszFileName)
//{
//
//}
//
//HBITMAP GetThumbnail(PTSTR lpszFileName)
//{
//	IShellItem *pFileItem = GetShellItem(lpszFileName);
//	IThumbnailCache *pThumbCache = nullptr;
//	HRESULT hr = CoCreateInstance(CLSID_LocalThumbnailCache, NULL,
//		CLSCTX_INPROC, IID_PPV_ARGS(&pThumbCache));
//	HBITMAP hThumbNail;
//	if (SUCCEEDED(hr))
//	{
//		ISharedBitmap* pShareBitmap = nullptr;
//		hr = pThumbCache->GetThumbnail(pFileItem, 100, WTS_NONE, &pShareBitmap, NULL, NULL);
//
//		if (pShareBitmap != nullptr)
//		{
//			pShareBitmap->GetSharedBitmap(&hThumbNail);
//			return hThumbNail;
//		}
//		else
//		{
//			TCHAR szBuffer[MAX_LOADSTRING] = _T("");
//			switch (hr)
//			{
//			case E_INVALIDARG:
//			{
//				StringCchCopy(szBuffer, MAX_LOADSTRING - 1, _T("A parameter is invalid."));
//			}
//			break;
//			case WTS_E_FAILEDEXTRACTION:
//			{
//				StringCchCopy(szBuffer, MAX_LOADSTRING - 1, _T("The Shell item does not support thumbnail extraction. For example, .exe or .lnk items."));
//			}
//			break;
//			case WTS_E_EXTRACTIONTIMEDOUT:
//			{
//				StringCchCopy(szBuffer, MAX_LOADSTRING - 1, _T("The extraction took longer than the maximum allowable time. The extraction was not completed."));
//			}
//			break;
//			case WTS_E_SURROGATEUNAVAILABLE:
//			{
//				StringCchCopy(szBuffer, MAX_LOADSTRING - 1, _T("A surrogate process was not available to be used for the extraction process."));
//			}
//			break;
//			case WTS_E_FASTEXTRACTIONNOTSUPPORTED:
//			{
//				StringCchCopy(szBuffer, MAX_LOADSTRING - 1, _T("The WTS_FASTEXTRACT flag was set, but fast extraction is not available."));
//			}
//			break;
//			}
//			StringCchCat(szBuffer, MAX_LOADSTRING - 1, _T(": "));
//			StringCchCat(szBuffer, MAX_LOADSTRING - 1, lpszFileName);
//		}
//	}
//	return NULL;
//}

//		The hien noi dung trong duong dan(global var vector<WIN32_FIND_DATA> ffd)	
BOOL ShowContent(HWND hWnd, TCHAR directory[])
{
	if (backed)
	{
		backed = FALSE;
	}
	TCHAR tempDir[MAX_PATH];
	StringCchCopy(tempDir, MAX_PATH, directory);
	if (wcslen(directory) == 3) // drives
	{
		StringCchCat(tempDir, MAX_PATH, _T("*"));
	}
	else
	{
		StringCchCat(tempDir, MAX_PATH, _T("\\*"));
	}
	WIN32_FIND_DATA temp;
	HANDLE hFind = FindFirstFile(tempDir, &temp);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		MessageBox(hWnd, _T("Directory don't exist."), _T("Error"), MB_OK);
		return FALSE;
	}
	ffd.clear();
	do
	{
		if (lstrcmp(_T(".."), temp.cFileName)!=0 && lstrcmp(_T("."), temp.cFileName) != 0)
		{
			ffd.push_back(temp);
		}
		uCountFindData++;
	} while (FindNextFile(hFind, &temp) != 0);
	
	SetWindowText(hWndTxtB_DIR, directory);
	FindClose(hFind);
	InitListViewItem(hWndLV);
	return TRUE;
}

/*		DISPLAY ERRORS		*/
void ReportError(LPCTSTR lpCaption)
{
	DWORD ee = GetLastError();
	LPTSTR Error = 0;
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		ee, 0, (LPTSTR)&Error, 0, NULL) != 0)
	{
		MessageBox(NULL, Error, lpCaption, MB_OK | MB_ICONERROR);
	}
	if (Error)
	{
		LocalFree(Error);
		Error = 0;
	}
}

/*		SUBCLASS FUNCTIONS		*/
//		Ham Sub Class Safe
void OnSafeSubclass(HWND hWnd, SUBCLASSPROC scProc, int iControl)
{
	HWND hButton = GetDlgItem(hWnd, iControl);
	UINT_PTR uIdSubclass = 0;
	if (!SetWindowSubclass(hButton, scProc, uIdSubclass, 0))
	{
		ReportError(L"OnSafeSubclass");
		return;
	}

	RECT rc;
	if (GetClientRect(hButton, &rc))
	{
		InvalidateRect(hButton, &rc, TRUE);
	}
}
//		Ham UnSub Class Safe
void OnSafeUnSubclass(HWND hWnd, SUBCLASSPROC scProc, int iControl)
{
	HWND hButton = GetDlgItem(hWnd, iControl);
	UINT_PTR uIdSubclass = 0;
	if (!RemoveWindowSubclass(hButton, scProc, uIdSubclass))
	{		
		return;
	}

	RECT rc;
	if (GetClientRect(hButton, &rc))
	{
		InvalidateRect(hButton, &rc, TRUE);
	}
}

/*		SUBCLASS CALLBACK PROCS		*/
//		listview
LRESULT CALLBACK LVProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData)
{
	int iSelected;
	switch (message) 
	{
	
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'R':	//rename
			if (GetAsyncKeyState(VK_CONTROL))
			{
				iSelected = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
				if (iSelected != -1 && ListView_GetNextItem(hwnd, iSelected, LVNI_SELECTED) == -1)
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, CreateFolderProc);
					TCHAR newName[100];
					StringCchCopy(newName, 100, lpszPassword);
					TCHAR szOld[MAX_PATH], szNew[MAX_PATH];
					StringCchCopy(szOld, 100, szDir);
					StringCchCopy(szNew, 100, szDir);
					if (wcslen(szDir) != 3) 
					{
						StringCchCat(szOld, 100, _T("\\"));
						StringCchCat(szNew, 100, _T("\\"));
					}
					StringCchCat(szOld, 100, ffd[iSelected].cFileName);
					StringCchCat(szNew, 100, newName);
					MoveFile(szOld, szNew);
					SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_BROWSE, 0);
				}
				
			}
			return TRUE;
		case 'C':	//copy
			if (GetAsyncKeyState(VK_CONTROL)) 
			{
				COPY = TRUE;
				ffdCoC.clear();
				StringCchCopy(szDirsrc, MAX_PATH, szDir);
				iSelected = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
				while (iSelected != -1)
				{
					ffdCoC.push_back(ffd[iSelected]);
					iSelected = ListView_GetNextItem(hwnd, iSelected, LVNI_SELECTED);
				}
			}
			return TRUE;
			break;
		case 'X':	//cut
			if (GetAsyncKeyState(VK_CONTROL))
			{
				COPY = FALSE;
				ffdCoC.clear();
				StringCchCopy(szDirsrc, MAX_PATH, szDir);
				iSelected = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
				while (iSelected != -1)
				{
					ffdCoC.push_back(ffd[iSelected]);
					iSelected = ListView_GetNextItem(hwnd, iSelected, LVNI_SELECTED);
				}
			}
			return TRUE;
			break;
		case 'V':	//paste
			if (GetAsyncKeyState(VK_CONTROL)) 
			{
				TCHAR path_src[MAX_PATH], path_des[MAX_PATH];
				StringCchCopy(path_src, MAX_PATH, szDirsrc);
				StringCchCopy(path_des, MAX_PATH, szDir);
				if (wcslen(path_src) != 3)
				{
					StringCchCat(path_src, MAX_PATH, _T("\\"));
				}
				if (wcslen(path_des) != 3)
				{
					StringCchCat(path_des, MAX_PATH, _T("\\"));
				}
				if (COPY)
				{
					for (int i = 0; i < ffdCoC.size(); i++)
					{
						TCHAR src[MAX_PATH], des[MAX_PATH];
						StringCchCopy(src, MAX_PATH, path_src);
						StringCchCat(src, MAX_PATH, ffdCoC[i].cFileName);
						StringCchCopy(des, MAX_PATH, path_des);
						StringCchCat(des, MAX_PATH, ffdCoC[i].cFileName);
						if (!CopyFile(src, des, TRUE))
						{
							TCHAR msgExist[300];
							StringCchCopy(msgExist, 300, _T("The "));
							StringCchCat(msgExist, 300, ffdCoC[i].cFileName);
							StringCchCat(msgExist, 300, _T(" file has already existed.\nWould you like to overwrite it?"));
							if (6 == MessageBox(hwnd, msgExist, _T("Error"), MB_YESNO))
							{
								CopyFile(src, des, FALSE);
							}
							else {
								MessageBox(hwnd, _T("File is not copied.", i), _T(""), MB_OK);
							}
						}
					}
				}
				else
				{
					for (int i = 1; i < CopyOrCut.size(); i++)
					{
						TCHAR src[MAX_PATH], des[MAX_PATH];
						StringCchCopy(src, MAX_PATH, path_src);
						StringCchCat(src, MAX_PATH, CopyOrCut[i]);
						StringCchCopy(des, MAX_PATH, path_des);
						StringCchCat(des, MAX_PATH, CopyOrCut[i]);
						MoveFile(src, des);
					}
				}
				SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_BROWSE, 0);
			}
			return TRUE;
			break;
		case VK_F5: //refresh
			SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_BROWSE, 0);
			return TRUE;
		case VK_DELETE:	//delete
			iSelected = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
			while (iSelected != -1)
			{
				TCHAR temp[MAX_PATH];
				StringCchCopy(temp, MAX_PATH, szDir);
				if (wcslen(temp) != 3) 
				{				
					StringCchCat(temp, MAX_PATH, _T("\\"));
				}
				StringCchCat(temp, MAX_PATH, ffd[iSelected].cFileName);
				if (DeleteFile(temp) == 0) 
				{
					RemoveDirectory(temp);
				}
				iSelected = ListView_GetNextItem(hwnd, iSelected, LVNI_SELECTED);
			}
			SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_BROWSE, 0);
			return TRUE;
		case VK_RETURN:
			iSelected = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
			if (-1 != iSelected	&&	ListView_GetNextItem(hwnd, iSelected, LVNI_SELECTED) == -1)	//chi chon 1 item
			{
				ListView_DeleteAllItems(hwnd);
				uCountFindData = 0;
				TCHAR temp[MAX_PATH];
				StringCchCopy(temp, MAX_PATH, szDir);
				if (wcslen(temp) != 3) 
				{
					StringCchCat(temp, MAX_PATH, _T("\\"));
				}
				StringCchCat(temp, MAX_PATH, ffd[iSelected].cFileName);
				if (!ShowContent(GetParent(hwnd), temp))
				{
					ShowContent(GetParent(hwnd), szDir);
				}
				else
				{
					StringCchCopy(szb4Dir, MAX_PATH, szDir);
					StringCchCopy(szDir, MAX_PATH, temp);
				}
			}
			else
			{
				MessageBox(GetParent(hwnd), _T("Can't open multiple folder."), _T("Error"), MB_OK);
			}

			return TRUE;
			break;
		}
		break;

	default:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	}
	return TRUE;
}

//		BrowseEBox
LRESULT CALLBACK BrowseProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_BROWSE, 0);		//gui lenh an nut Browse cho handle cua window chinh
			return TRUE;
			break;
		}
		return TRUE;
		break;
	default:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	}
	return TRUE;
}

//		SearchBox
LRESULT CALLBACK SearchProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			SendMessage(GetParent(hwnd), WM_COMMAND, BUTTON_SEARCH, 0);		//gui lenh an nut Search cho handle cua window chinh
			return TRUE;
		}
		return TRUE;

	default:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	}
	return TRUE;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BTTRENLOP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BTTRENLOP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int iSelected;
    switch (message)
    {
	case WM_CREATE:		
		/*		Tao Controls		*/
		//		EditControls
		hWndTxtB_DIR = CreateWindow(L"EDIT", L"", WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW, 150, 50, 400, 20, hWnd, (HMENU)ID_BROWSE, NULL, NULL);		//Browse
		hWndTxtB_FILE = CreateWindow(L"EDIT", L"", WS_BORDER | WS_VISIBLE | WS_CHILDWINDOW, 640, 50, 200, 20, hWnd, (HMENU)ID_SEARCH, NULL, NULL);		//Search

		//ButtonControls
		hWndBtt_Browse = CreateWindow(L"BUTTON", L"Browse", WS_BORDER | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,		//Browse
			560, 50, 60, 20, hWnd, (HMENU)BUTTON_BROWSE, NULL, NULL);
		hWndBtt_Search = CreateWindow(L"BUTTON", L"Search", WS_BORDER | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,		//Search
			850, 50, 60, 20, hWnd, (HMENU)BUTTON_SEARCH, NULL, NULL);

		// LVControl
		hWndLV = CreateListView(hWnd);
		if (hWndLV == NULL) {
			MessageBox(NULL, _T("Listview not created!"), NULL, MB_OK);
		}
		InitListViewCol(hWndLV);


		/*		Tao SubClass		*/	
		OnSafeSubclass(hWnd, LVProc, IDM_CODE_SAMPLES);		//LV
		OnSafeSubclass(hWnd, BrowseProc, ID_BROWSE);		//Browse
		OnSafeSubclass(hWnd, SearchProc, ID_SEARCH);		//Search

		/*		Mo cac o dia va desktop		*/

		break;
	case WM_NOTIFY:
	{
		switch (LOWORD(wParam))
		{
		case IDM_CODE_SAMPLES:
			if (((LPNMHDR)lParam)->code == NM_DBLCLK)
			{
				int iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
				if (-1 != iSelected	&&	ListView_GetNextItem(hWndLV, iSelected, LVNI_SELECTED) == -1)	//chi chon 1 item
				{
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					TCHAR temp[MAX_PATH];
					StringCchCopy(temp, MAX_PATH, szDir);
					if (wcslen(temp) != 3)
						StringCchCat(temp, MAX_PATH, _T("\\"));
					StringCchCat(temp, MAX_PATH, ffd[iSelected].cFileName);
					if (!ShowContent(hWnd, temp))
					{
						ShowContent(hWnd, szDir);
					}
					else
					{
						StringCchCopy(szb4Dir, MAX_PATH, szDir);
						StringCchCopy(szDir, MAX_PATH, temp);
					}
				}
				else
				{
					MessageBox(hWnd, _T("Can't open multiple folder."), _T("Error"), MB_OK);
				}				
			}
			if ((((LPNMHDR)lParam)->idFrom == IDM_CODE_SAMPLES) && (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK))
			{
				MessageBox(hWnd, _T("Clicked"), _T("Error"), MB_OK);
				OnColumnClick((LPNMLISTVIEW)lParam);
			}
			break;
		}												
	}
		break;
	case WM_KEYDOWN:
		switch (wParam) 
		{
		case 'N':
			if (GetAsyncKeyState(VK_CONTROL)) {
				SendMessage(hWnd, WM_COMMAND, ID_FILE_NEW, 0);
			}
			break;
			// more keys here
		}
		break;
	case VK_F5:
		SendMessage(hWnd, WM_COMMAND, BUTTON_BROWSE, 0);
		break;
    case WM_COMMAND:
        {
            int				wmId = LOWORD(wParam);
			BROWSEINFO		bi = { 0 };
			int				DIRLen = 0;
			LPITEMIDLIST	pidl;
			//LARGE_INTEGER	filesize;
			
			//TCHAR			TEMP[100];
			//size_t			length_of_arg;
			HANDLE			hFind = INVALID_HANDLE_VALUE;
			DWORD			dwError = 0;
			//WIN32_FIND_DATA temp;
			// Parse the menu selections:
			

            switch (wmId)
            {
			case BUTTON_BROWSE:
			{
				DIRLen = GetWindowTextLength(hWndTxtB_DIR);
				if (DIRLen != 0) {
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					StringCchCopy(szb4Dir, MAX_PATH, szDir);
					GetWindowText(hWndTxtB_DIR, szDir, MAX_PATH);
					ShowContent(hWnd, szDir);
				}
				else
				{
					SendMessage(hWnd, WM_COMMAND, ID_FILE_BROWSE, 0);
				}

			}
			break;
			case ID_FILE_COPY:
				COPY = TRUE;
				ffdCoC.clear();
				StringCchCopy(szDirsrc, MAX_PATH, szDir);
				iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
				while (iSelected != -1)
				{
					ffdCoC.push_back(ffd[iSelected]);
					iSelected = ListView_GetNextItem(hWndLV, iSelected, LVNI_SELECTED);
				}
				break;
			case ID_FILE_CUT:
				COPY = FALSE;
				ffdCoC.clear();
				StringCchCopy(szDirsrc, MAX_PATH, szDir);
				iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
				while (iSelected != -1)
				{
					ffdCoC.push_back(ffd[iSelected]);
					iSelected = ListView_GetNextItem(hWndLV, iSelected, LVNI_SELECTED);
				}
				break;
			case ID_FILE_PASTE:
				TCHAR path_src[MAX_PATH], path_des[MAX_PATH];
				StringCchCopy(path_src, MAX_PATH, szDirsrc);
				StringCchCopy(path_des, MAX_PATH, szDir);
				if (wcslen(path_src) != 3)
				{
					StringCchCat(path_src, MAX_PATH, _T("\\"));
				}
				if (wcslen(path_des) != 3)
				{
					StringCchCat(path_des, MAX_PATH, _T("\\"));
				}
				if (COPY)
				{
					for (int i = 0; i < ffdCoC.size(); i++)
					{
						TCHAR src[MAX_PATH], des[MAX_PATH];
						StringCchCopy(src, MAX_PATH, path_src);
						StringCchCat(src, MAX_PATH, ffdCoC[i].cFileName);
						StringCchCopy(des, MAX_PATH, path_des);
						StringCchCat(des, MAX_PATH, ffdCoC[i].cFileName);
						if (!CopyFile(src, des, TRUE))
						{
							TCHAR msgExist[300];
							StringCchCopy(msgExist, 300, _T("The "));
							StringCchCat(msgExist, 300, ffdCoC[i].cFileName);
							StringCchCat(msgExist, 300, _T(" file has already existed.\nWould you like to overwrite it?"));
							if (6 == MessageBox(hWnd, msgExist, _T("Error"), MB_YESNO))
							{
								CopyFile(src, des, FALSE);
							}
							else {
								MessageBox(hWnd, _T("File is not copied.", i), _T(""), MB_OK);
							}
						}
					}
				}
				else
				{
					for (int i = 1; i < CopyOrCut.size(); i++)
					{
						TCHAR src[MAX_PATH], des[MAX_PATH];
						StringCchCopy(src, MAX_PATH, path_src);
						StringCchCat(src, MAX_PATH, CopyOrCut[i]);
						StringCchCopy(des, MAX_PATH, path_des);
						StringCchCat(des, MAX_PATH, CopyOrCut[i]);
						MoveFile(src, des);
					}
				}
				SendMessage(hWnd, WM_COMMAND, BUTTON_BROWSE, 0);
				break;
			case ID_VIEW_REPORT:
			{
				SetWindowLong(hWndLV, GWL_STYLE, (LONG)(WS_CHILD | LVS_REPORT | WS_VISIBLE));
			}
			break;
			case ID_VIEW_ICON:
			{
				SetWindowLong(hWndLV, GWL_STYLE,(LONG)(WS_CHILD | LVS_ICON | WS_VISIBLE));
			}
			break;
			case ID_VIEW_LIST:
			{
				SetWindowLong(hWndLV, GWL_STYLE, (LONG)(WS_CHILD | LVS_LIST | WS_VISIBLE));
			}
			break;
			case BUTTON_SEARCH:
			{
				MessageBox(hWnd, szb4Dir, szDir, MB_OK);
				//TCHAR Pattern[1000], SearchTerm[MAX_PATH];
				//StringCchCopy(SearchTerm, MAX_PATH, szDir);
				//GetWindowText(hWndTxtB_FILE, Pattern, MAX_PATH);
				//if (sizeof(szDir) != 3) {
				//	//StringCchCat(SearchTerm, MAX_PATH, _T("\\"));
				//}
				//StringCchCat(SearchTerm, MAX_PATH, Pattern);
				//MessageBox(hWnd, SearchTerm, szDir, MB_OK);
				//WIN32_FIND_DATA ffd_Found;
				//HANDLE hFind_Search;
				//hFind_Search = FindFirstFileEx(szDir, FindExInfoStandard, &ffd_Found,	FindExSearchNameMatch, NULL, 0);
				//if (hFind == INVALID_HANDLE_VALUE)
				//{
				//	MessageBox(hWnd, _T("Found nothing"), _T("Error"), MB_OK);
				//}
				//else
				//{
				//	MessageBox(hWnd, ffd_Found.cFileName, ffd_Found.cFileName, MB_OK);
				//}
			}
			break; 
			case ID_FILE_NEW:
			{
				if (wcslen(szDir) == 0)
					break;
				TCHAR newDir[MAX_PATH];
				StringCchCopy(newDir, MAX_PATH, szDir);
				if (wcslen(newDir) != 3)
				{
					StringCchCat(newDir, MAX_PATH, _T("\\"));
				}
				
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, CreateFolderProc);
				StringCchCat(newDir, MAX_PATH, lpszPassword);
				

				if (CreateDirectory(newDir, NULL) ||
					ERROR_ALREADY_EXISTS == GetLastError())
				{
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					ShowContent(hWnd, szDir);
				}
				else
				{
					MessageBox(hWnd, _T("Fail to create new folder"), _T("Error"), MB_OK);
				}
				
			}
				break;
			case ID_NAVIGATE_BACK:
				if (!backed)
				{
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					backed = TRUE;
					StringCchCopy(szafDir, MAX_PATH, szDir);
					StringCchCopy(szDir, MAX_PATH, szb4Dir);
					ShowContent(hWnd, szDir);
				}
				break;
			case ID_NAVIGATE_FORTH:
				if (backed)
				{
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					backed = FALSE;
					StringCchCopy(szb4Dir, MAX_PATH, szDir);
					StringCchCopy(szDir, MAX_PATH, szafDir);
					ShowContent(hWnd, szDir);
				}
				break;
			case ID_FILE_RENAME:
				iSelected = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);
				if (iSelected != -1 && ListView_GetNextItem(hWndLV, iSelected, LVNI_SELECTED) == -1)
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, CreateFolderProc);
					TCHAR newName[100];
					StringCchCopy(newName, 100, lpszPassword);
					TCHAR szOld[MAX_PATH], szNew[MAX_PATH];
					StringCchCopy(szOld, 100, szDir);
					StringCchCopy(szNew, 100, szDir);
					if (wcslen(szDir) != 3)
					{
						StringCchCat(szOld, 100, _T("\\"));
						StringCchCat(szNew, 100, _T("\\"));
					}
					StringCchCat(szOld, 100, ffd[iSelected].cFileName);
					StringCchCat(szNew, 100, newName);
					MessageBox(hWnd, szOld, szNew, MB_OK);
					MoveFile(szOld, szNew);
					SendMessage(hWnd, WM_COMMAND, BUTTON_BROWSE, 0);
				}
				break;
			case ID_FILE_BROWSE:				
				bi.lpszTitle = _T("Choose a folder");
				pidl = SHBrowseForFolder(&bi);
				if (pidl != 0)
				{
					ListView_DeleteAllItems(hWndLV);
					uCountFindData = 0;
					// get the name of the folder
					TCHAR path[MAX_PATH];
					SHGetPathFromIDList(pidl, path);

					// free memory used
					/*IMalloc * imalloc = 0;
					if (SUCCEEDED(SHGetMalloc(&imalloc)))
					{
						imalloc->Free(pidl);
						imalloc->Release();
					}*/
					if (wcslen(path) != 0)
					{
						StringCchCopy(szb4Dir, MAX_PATH, path);
						StringCchCopy(szDir, MAX_PATH, path);
						ShowContent(hWnd, szDir);
						SetWindowText(hWndTxtB_DIR, path);
					}
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
				OnSafeUnSubclass(hWnd, LVProc, IDM_CODE_SAMPLES);					//LV
				OnSafeUnSubclass(hWnd, BrowseProc, GetDlgCtrlID(hWndTxtB_DIR));		//Browse
				OnSafeUnSubclass(hWnd, SearchProc, GetDlgCtrlID(hWndTxtB_DIR));		//Search
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		OnSafeUnSubclass(hWnd, LVProc, IDM_CODE_SAMPLES);					//LV
		OnSafeUnSubclass(hWnd, BrowseProc, GetDlgCtrlID(hWndTxtB_DIR));		//Browse
		OnSafeUnSubclass(hWnd, SearchProc, GetDlgCtrlID(hWndTxtB_DIR));		//Search
        PostQuitMessage(0);
        break;
    default:
		
        return DefWindowProc(hWnd, message, wParam, lParam);
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


