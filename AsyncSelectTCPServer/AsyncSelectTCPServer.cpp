#include "TCPServer.h"

TCPServer tcpServer;

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

int main(int argc, char *argv[])
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
	HWND hWnd = CreateWindow(_T("MyWndClass"), _T("TCP 서버"),
		WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, NULL, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	//소켓 준비작업
	tcpServer.Init(hWnd);

	// 메시지 루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_SOCKET: // 소켓 관련 윈도우 메시지
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 소켓 관련 메시지 처리
void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 데이터 통신에 사용할 변수
	int retval;
	SOCKET client_sock ;
	struct sockaddr_in clientaddr;
	int addrlen{};
	SOCKETINFO *ptr =nullptr;

	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam)) {
		err_display(WSAGETSELECTERROR(lParam));
		tcpServer.RemoveSocketInfo(wParam);
		return;
	}

	// 메시지 처리
	switch (WSAGETSELECTEVENT(lParam)) {
	case FD_ACCEPT:
		tcpServer.AcceptProcess(hWnd, client_sock, addrlen, clientaddr, wParam);
		break;
	case FD_READ:
		tcpServer.ReadProcess(hWnd, addrlen, clientaddr, wParam);
	case FD_WRITE:
		tcpServer.WriteProcess(hWnd, addrlen, clientaddr, wParam);
		break;
	case FD_CLOSE:
		tcpServer.RemoveSocketInfo(wParam);
		break;
	}
}
