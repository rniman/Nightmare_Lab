#include "stdafx.h"
#include "TCPServer.h"

TCPServer g_tcpServer;
HINSTANCE hInst;                  

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnProcessingSocketMessage(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int main()
{
	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T("MyWndClass");
	if (!RegisterClass(&wndclass)) return 1;
	
	// 윈도우 생성
	HWND hWnd = CreateWindow(_T("MyWndClass"), _T("TCP 서버"), WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, NULL, NULL, NULL, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	HWND hListBox = CreateWindowEx(
		0,
		L"LISTBOX",              // 클래스 이름
		L"",                       // 콤보 박스에 표시되는 텍스트 (처음에는 비어 있음)
		WS_VISIBLE | WS_CHILD | LBS_STANDARD, // 스타일
		10, 10, 400, 200,          // 위치와 크기
		hWnd,                      // 부모 윈도우 핸들
		NULL,                      // 메뉴 핸들
		NULL,						// 인스턴스 핸들
		NULL                       // 추가 파라미터
	);

	//소켓 준비작업
	g_tcpServer.Init(hWnd);
	g_tcpServer.SetClientListBox(hListBox);

	MSG msg;
	// 메시지 루프
	while (1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			g_tcpServer.SimulationLoop();
		}
	}

	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch (uMsg) 
	{
	case WM_ACTIVATE:
	case WM_SOUND:
		OnProcessingWindowMessage(hWnd, uMsg, wParam, lParam);
		break;
	case WM_SOCKET: // 소켓 관련 윈도우 메시지
		return OnProcessingSocketMessage(hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
	case WM_SOUND:
		g_tcpServer.OnProcessingWindowMessage(hWnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}
// 소켓 관련 메시지 처리
LRESULT CALLBACK OnProcessingSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam) && WSAGETSELECTEVENT(lParam) != FD_CLOSE) 
	{
		err_display(WSAGETSELECTERROR(lParam));
		int nIndex = g_tcpServer.RemoveSocketInfo(wParam);
		g_tcpServer.GetPlayer(nIndex).reset();
		if (nIndex == ZOMBIEPLAYER)
		{
			int nZombie = g_tcpServer.GetNumOfZombie();
			g_tcpServer.SetNumOfZombie(nZombie-1);
		}
		else
		{
			int nBlueSuit = g_tcpServer.GetNumOfBlueSuit();
			g_tcpServer.SetNumOfBlueSuit(nBlueSuit - 1);
		}
		return -1;
	}

	// 메시지 처리
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
	case FD_READ:
	case FD_WRITE:		
	case FD_CLOSE:
		g_tcpServer.OnProcessingSocketMessage(hWnd, uMsg, wParam, lParam);
		break;
	default:
		break;
	}

	return 0;
}
