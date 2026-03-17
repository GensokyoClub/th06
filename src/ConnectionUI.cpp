#include "ConnectionUI.hpp"
#include <cstdlib>
#include <sstream>

#define IDC_EDIT_HOST_IP 1001
#define IDC_EDIT_HOST_PORT 1002
#define IDC_EDIT_LISTEN_PORT 1003
#define IDC_BTN_START_HOST 1004
#define IDC_BTN_START_GUEST 1005
#define IDC_STATIC_LATENCY 1006
#define IDC_EDIT_TARGET_LATENCY 1007
#define IDC_BTN_START_GAME 1008
#define IDC_CHECKBOX_IS_HOST_1P 1009
#define IDC_BTN_START_GAME_LOCAL 1010

#define TIMER_ID_POLL 1
#define TIMER_INTERVAL_MS 50

ULONGLONG MyGetTickCount()
{
    LARGE_INTEGER l;
    static LARGE_INTEGER f;
    static bool is_inited = false;
    if (!is_inited)
    {
        is_inited = true;
        QueryPerformanceFrequency(&f);
    }
    QueryPerformanceCounter(&l);
    return l.QuadPart * 1000 / f.QuadPart;
}

ConnectionUI::ConnectionUI(Host &h, Guest &g) : m_host(h), m_guest(g)
{
    m_isHost = false;
    m_isGuest = false;
    m_connected = false;
    m_startGame = false;

    m_hWnd = NULL;
    m_editHostIp = NULL;
    m_editHostPort = NULL;
    m_editListenPort = NULL;
    m_btnHost = NULL;
    m_btnGuest = NULL;
    m_staticLatency = NULL;
    m_editTargetLatency = NULL;
    m_btnStartGame = NULL;

    m_guestWaitStartTick = 0;
    m_lastPeriodicPingTick = 0;
    m_seq = 1;
}

ConnectionUI::~ConnectionUI()
{
}

int ConnectionUI::GetDelay()
{
    return m_delay;
}

bool ConnectionUI::GetIsHostP1()
{
    return m_is_host_p1;
}
void ConnectionUI::SetDelay(int delay)
{
    m_delay = delay;
    if (m_delay <= 0)
    {
        m_delay = 1;
    }
    if (m_delay > 60)
    {
        m_delay = 60;
    }
    char chs[60];
    sprintf(chs, "%d", m_delay);
    SetWindowTextA(m_editTargetLatency, chs);
    return;
}

void ConnectionUI::SetIsHostP1(bool is_host_p1)
{
    m_is_host_p1 = is_host_p1;
    SendMessage(m_checkBoxIsHost1P, BM_SETCHECK, m_is_host_p1 ? BST_CHECKED : BST_UNCHECKED, 0);
    return;
}

bool ConnectionUI::IsHost() const
{
    return m_isHost;
}

bool ConnectionUI::IsGuest() const
{
    return m_isGuest;
}

std::string ConnectionUI::GetEditText(HWND hEdit)
{
    char buf[256] = {0};
    GetWindowTextA(hEdit, buf, sizeof(buf));
    return std::string(buf);
}

int ConnectionUI::GetEditInt(HWND hEdit)
{
    char buf[64] = {0};
    GetWindowTextA(hEdit, buf, sizeof(buf));
    int n = strlen(buf);
    for (int i = 0; i < n; i++)
        if (buf[i] > '9' || buf[i] < '0')
        {
            MessageBoxA(NULL, "wrong number", "err", MB_OK);
            return -1;
        }
    return atoi(buf);
}

void ConnectionUI::SetText(HWND hWnd, const std::string &s)
{
    SetWindowTextA(hWnd, s.c_str());
}

void ConnectionUI::SetLatencyText(const std::string &s)
{
    SetText(m_staticLatency, s);
}

std::string ConnectionUI::BuildLatencyText(const std::string &ip, int port, ULONGLONG rtt)
{
    std::ostringstream oss;
    oss << ip << ":" << port << "(" << (ULONGLONG)rtt / 2 << "ms)";
    return oss.str();
}

bool ConnectionUI::CreateMainWindow(HINSTANCE hInst)
{
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = ConnectionUI::WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "ConnectionUIClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);

    m_hWnd =
        CreateWindowA("ConnectionUIClass", "Game Launcher", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                      CW_USEDEFAULT, CW_USEDEFAULT, 430, 480, NULL, NULL, hInst, this);

    return (m_hWnd != NULL);
}

const char *g_iniPath = ".\\connect_config.ini";

void ConnectionUI::SaveControls()
{
    char buf[128];
    GetWindowTextA(m_editHostIp, buf, 128);
    WritePrivateProfileStringA("Connection", "ip", buf, g_iniPath);

    GetWindowTextA(m_editHostPort, buf, 128);
    WritePrivateProfileStringA("Connection", "port_host", buf, g_iniPath);

    GetWindowTextA(m_editListenPort, buf, 128);
    WritePrivateProfileStringA("Connection", "port_listen", buf, g_iniPath);

    GetWindowTextA(m_editTargetLatency, buf, 128);
    WritePrivateProfileStringA("Connection", "target_delay", buf, g_iniPath);
    WritePrivateProfileStringA("Connection", "is_host_p1", GetIsHostP1() ? "1" : "0", g_iniPath);
}

void ConnectionUI::CreateControls(HWND hWnd)
{
    static char ip[128];
    static char port_host[128];
    static char port_listen[128];
    static char target_delay[128];
    static bool is_host_p1;

    GetPrivateProfileStringA("Connection", "ip", "::1", ip, sizeof(ip), g_iniPath);
    GetPrivateProfileStringA("Connection", "port_host", "3036", port_host, sizeof(port_host), g_iniPath);
    GetPrivateProfileStringA("Connection", "port_listen", "3036", port_listen, sizeof(port_listen), g_iniPath);
    GetPrivateProfileStringA("Connection", "target_delay", "2", target_delay, sizeof(target_delay), g_iniPath);
    is_host_p1 = GetPrivateProfileIntA("Connection", "is_host_p1", 0, g_iniPath);

    CreateWindowA("STATIC", "Host IP:", WS_CHILD | WS_VISIBLE, 20, 20, 80, 20, hWnd, NULL, NULL, NULL);

    m_editHostIp = CreateWindowA("EDIT", ip, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 110, 20, 270, 24, hWnd,
                                 (HMENU)IDC_EDIT_HOST_IP, NULL, NULL);

    CreateWindowA("STATIC", "Host Port:", WS_CHILD | WS_VISIBLE, 20, 60, 80, 20, hWnd, NULL, NULL, NULL);

    m_editHostPort = CreateWindowA("EDIT", port_host, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
                                   110, 60, 100, 24, hWnd, (HMENU)IDC_EDIT_HOST_PORT, NULL, NULL);

    CreateWindowA("STATIC", "Listen Port:", WS_CHILD | WS_VISIBLE, 20, 100, 80, 20, hWnd, NULL, NULL, NULL);

    m_editListenPort =
        CreateWindowA("EDIT", port_listen, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 110, 100,
                      100, 24, hWnd, (HMENU)IDC_EDIT_LISTEN_PORT, NULL, NULL);

    m_btnHost = CreateWindowA("BUTTON", "as host", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 150, 160, 32, hWnd,
                              (HMENU)IDC_BTN_START_HOST, NULL, NULL);

    m_btnGuest = CreateWindowA("BUTTON", "as guest", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 150, 160, 32, hWnd,
                               (HMENU)IDC_BTN_START_GUEST, NULL, NULL);

    CreateWindowA("STATIC", "cur state:", WS_CHILD | WS_VISIBLE, 20, 210, 80, 20, hWnd, NULL, NULL, NULL);

    m_staticLatency = CreateWindowA("STATIC", "no connection", WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 210, 270, 24,
                                    hWnd, (HMENU)IDC_STATIC_LATENCY, NULL, NULL);

    CreateWindowA("STATIC", "target delay:", WS_CHILD | WS_VISIBLE, 20, 250, 120, 20, hWnd, NULL, NULL, NULL);

    m_editTargetLatency =
        CreateWindowA("EDIT", target_delay, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 110, 250,
                      100, 24, hWnd, (HMENU)IDC_EDIT_TARGET_LATENCY, NULL, NULL);

    m_checkBoxIsHost1P = CreateWindowA("BUTTON", "Host is 1P", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 20, 310, 100,
                                       24, hWnd, (HMENU)IDC_CHECKBOX_IS_HOST_1P, NULL, NULL);

    m_btnStartGame = CreateWindowA("BUTTON", "Start Game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED, 20, 340,
                                   180, 36, hWnd, (HMENU)IDC_BTN_START_GAME, NULL, NULL);

    m_btnStartGameLocal = CreateWindowA("BUTTON", "Start Game(local)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 340,
                                        180, 36, hWnd, (HMENU)IDC_BTN_START_GAME_LOCAL, NULL, NULL);

    m_delay = atoi(target_delay);
    if (m_delay <= 0)
    {
        m_delay = 1;
        SetWindowTextA(m_editTargetLatency, "1");
    }
    if (m_delay > 60)
    {
        m_delay = 60;
        SetWindowTextA(m_editTargetLatency, "60");
    };

    m_is_host_p1 = is_host_p1;
    if (is_host_p1)
    {
        SendMessage(m_checkBoxIsHost1P, BM_SETCHECK, is_host_p1 ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool ConnectionUI::IsGameStarted()
{
    return this->m_startGame;
}
bool ConnectionUI::IsConnected()
{
    return m_connected;
}

bool ConnectionUI::TryStartHost(int listenPort)
{
    return m_host.Start("", listenPort, AF_INET6);
}

bool ConnectionUI::TryStartGuest(const std::string &hostIp, int hostPort, int listenPort)
{
    int family = (hostIp.find(':') != std::string::npos) ? AF_INET6 : AF_INET;
    return m_guest.Start(hostIp, hostPort, listenPort, family);
}

void ConnectionUI::EnterHostWaitingState()
{
    m_isHost = true;
    m_isGuest = false;
    m_connected = false;
    m_lastPeriodicPingTick = 0;

    EnableWindow(m_btnGuest, FALSE);
    SetText(m_btnHost, "waiting guest");
    SetLatencyText("waiting guest...");
}

void ConnectionUI::EnterGuestWaitingState()
{
    m_isHost = false;
    m_isGuest = true;
    m_connected = false;
    m_guestWaitStartTick = MyGetTickCount();
    m_lastPeriodicPingTick = 0;

    EnableWindow(m_btnHost, FALSE);
    SetText(m_btnGuest, "waiting msg...");
    SetLatencyText("trying connection...");
}

void ConnectionUI::EnterConnectedState()
{
    if (m_connected)
        return;

    m_connected = true;
    m_lastPeriodicPingTick = 0;

    EnableWindow(m_btnStartGame, TRUE);

    if (m_isHost)
        SetText(m_btnHost, "connected");
    else if (m_isGuest)
        SetText(m_btnGuest, "connected");
}

void ConnectionUI::ResetGuestButtonAfterTimeout()
{
    m_guest.Reset();
    SetText(m_btnGuest, "as guest");
    EnableWindow(m_btnHost, TRUE);
    SetLatencyText("no connection");
    m_isGuest = false;
    m_connected = false;
}

void ConnectionUI::SendPingAsHost(CtrlPack pctrl)
{
    Pack p;
    p.ctrl = pctrl;
    p.type = PACK_PING;
    p.seq = m_seq++;
    p.sendTick = MyGetTickCount();
    p.echoTick = 0;

    m_host.SendPack(p);
}

void ConnectionUI::SendPingAsGuest(CtrlPack pctrl)
{
    Pack p;
    p.ctrl = pctrl;
    p.type = PACK_PING;
    p.seq = m_seq++;
    p.sendTick = MyGetTickCount();
    p.echoTick = 0;

    m_guest.SendPack(p);
}

void ConnectionUI::TryPeriodicPing()
{
    if (!m_connected)
        return;

    ULONGLONG now = MyGetTickCount();
    if (m_lastPeriodicPingTick == 0 || now - m_lastPeriodicPingTick >= 1000)
    {
        if (m_isHost)
        {
            CtrlPack c;
            c.ctrl_type = Ctrl_Set_InitSetting;
            c.init_setting.delay = GetDelay();
            c.init_setting.is_host_p1 = GetIsHostP1();
            SendPingAsHost(c);
        }
        else if (m_isGuest)
            SendPingAsGuest(CtrlPack());

        m_lastPeriodicPingTick = now;
    }
}

void ConnectionUI::OnClickHost()
{
    int listenPort = GetEditInt(m_editListenPort);
    if (listenPort == -1)
        return;
    if (!TryStartHost(listenPort))
    {
        MessageBoxA(m_hWnd, "fail to start as host", "err", MB_OK | MB_ICONERROR);
        return;
    }

    EnterHostWaitingState();
}

void ConnectionUI::OnClickGuest()
{
    std::string hostIp = GetEditText(m_editHostIp);
    int hostPort = GetEditInt(m_editHostPort);
    int listenPort = GetEditInt(m_editListenPort);
    EnableWindow(m_editTargetLatency, FALSE);
    EnableWindow(m_checkBoxIsHost1P, FALSE);

    if (listenPort == -1 || hostPort == -1)
        return;

    if (!TryStartGuest(hostIp, hostPort, listenPort))
    {
        MessageBoxA(m_hWnd, "fail to start as guest", "err", MB_OK | MB_ICONERROR);
        return;
    }

    EnterGuestWaitingState();

    // ping
    SendPingAsGuest(CtrlPack());
}

void ConnectionUI::OnClickStartGame()
{
    if (!m_connected)
        return;
    m_startGame = true;
    CtrlPack p;
    p.ctrl_type = Ctrl_Start_Game;
    p.init_setting.delay = GetDelay();
    p.init_setting.is_host_p1 = GetIsHostP1();
    if (this->IsHost())
        SendPingAsHost(p);
    else
        SendPingAsGuest(p);
    return;

    // DestroyWindow(m_hWnd);
}

void ConnectionUI::ProcessHostNetwork()
{
    while (true)
    {
        Pack p;
        bool hasData = false;

        if (!m_host.PollReceive(p, hasData))
            break;

        if (!hasData)
            break;

        // host pong
        if (p.type == PACK_PING)
        {
            Pack reply;
            reply.type = PACK_PONG;
            reply.seq = p.seq;
            reply.sendTick = p.sendTick; // cal RTT
            reply.echoTick = MyGetTickCount();
            reply.ctrl = p.ctrl;

            m_host.SendPack(reply);

            if (!m_connected)
                EnterConnectedState();
            if (p.ctrl.ctrl_type == Ctrl_Start_Game)
            {
                m_startGame = true;
                DestroyWindow(m_hWnd);
            }
        }
        // host pong2
        else if (p.type == PACK_PONG)
        {
            ULONGLONG now = MyGetTickCount();
            ULONGLONG rtt = now - p.sendTick;
            SetLatencyText(BuildLatencyText(m_host.GetGuestIp(), m_host.GetGuestPort(), rtt));

            if (!m_connected)
                EnterConnectedState();
            if (p.ctrl.ctrl_type == Ctrl_Start_Game)
            {
                DestroyWindow(m_hWnd);
            }
        }
    }
}

void ConnectionUI::ProcessGuestNetwork()
{
    bool gotAnyData = false;

    while (true)
    {
        Pack p;
        bool hasData = false;

        if (!m_guest.PollReceive(p, hasData))
            break;

        if (!hasData)
            break;

        gotAnyData = true;

        // guest rcv ping
        if (p.type == PACK_PING)
        {
            Pack reply;
            reply.type = PACK_PONG;
            reply.seq = p.seq;
            reply.sendTick = p.sendTick;
            reply.echoTick = MyGetTickCount();
            reply.ctrl = p.ctrl;

            m_guest.SendPack(reply);

            if (!m_connected)
                EnterConnectedState();

            if (p.ctrl.ctrl_type == Ctrl_Start_Game)
            {
                m_startGame = true;
                DestroyWindow(m_hWnd);
            }
            else if (p.ctrl.ctrl_type == Ctrl_Set_InitSetting)
            {
                SetDelay(p.ctrl.init_setting.delay);
                SetIsHostP1(p.ctrl.init_setting.is_host_p1);
            }
        }
        // guest rcv pong
        else if (p.type == PACK_PONG)
        {
            ULONGLONG now = MyGetTickCount();
            ULONGLONG rtt = now - p.sendTick;
            SetLatencyText(BuildLatencyText(m_guest.GetHostIp(), m_guest.GetHostPort(), rtt));

            if (!m_connected)
                EnterConnectedState();

            if (p.ctrl.ctrl_type == Ctrl_Start_Game)
            {
                DestroyWindow(m_hWnd);
            }
        }
    }

    // guest waiting
    if (!m_connected)
    {
        ULONGLONG now = MyGetTickCount();
        if (!gotAnyData && now - m_guestWaitStartTick > 1000)
        {
            ResetGuestButtonAfterTimeout();
            MessageBoxA(m_hWnd, "no connection", "warning", MB_OK | MB_ICONWARNING);
        }
    }
}

void ConnectionUI::OnTimer()
{
    if (m_isHost)
    {
        ProcessHostNetwork();
        TryPeriodicPing();
    }
    else if (m_isGuest)
    {
        ProcessGuestNetwork();
        TryPeriodicPing();
    }
}

LRESULT CALLBACK ConnectionUI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ConnectionUI *pThis = NULL;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTA *pcs = (CREATESTRUCTA *)lParam;
        pThis = (ConnectionUI *)pcs->lpCreateParams;
        SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hWnd;
    }
    else
    {
        pThis = (ConnectionUI *)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    }

    if (pThis)
        return pThis->HandleMessage(hWnd, msg, wParam, lParam);

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

LRESULT ConnectionUI::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        CreateControls(hWnd);
        SetTimer(hWnd, TIMER_ID_POLL, TIMER_INTERVAL_MS, NULL);
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);

        switch (id)
        {
        case IDC_BTN_START_HOST:
            OnClickHost();
            return 0;

        case IDC_BTN_START_GUEST:
            OnClickGuest();
            return 0;

        case IDC_BTN_START_GAME:
            OnClickStartGame();
            return 0;
        case IDC_BTN_START_GAME_LOCAL:
            m_startGame = true;
            m_connected = false;
            DestroyWindow(m_hWnd);
            return 0;
        case IDC_EDIT_HOST_IP:
        case IDC_EDIT_HOST_PORT:
        case IDC_EDIT_LISTEN_PORT:
            break;
        case IDC_EDIT_TARGET_LATENCY:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                char buf[32];
                GetWindowTextA(m_editTargetLatency, buf, 16);
                m_delay = atoi(buf);
                if (m_delay <= 0)
                {
                    m_delay = 1;
                    SetWindowTextA(m_editTargetLatency, "1");
                }
                if (m_delay > 60)
                {
                    m_delay = 60;
                    SetWindowTextA(m_editTargetLatency, "60");
                    SendMessage((HWND)lParam, EM_SETSEL, 2, 2);
                }
                if (IsHost())
                    TryPeriodicPing();
            }

            break;
        case IDC_CHECKBOX_IS_HOST_1P: {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                LRESULT state = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
                m_is_host_p1 = (state == BST_CHECKED);
                if (IsHost())
                    TryPeriodicPing();
            }

            break;
        }
        }
        break;
    }

    case WM_TIMER:
        if (wParam == TIMER_ID_POLL)
        {
            OnTimer();
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        SaveControls();
        KillTimer(hWnd, TIMER_ID_POLL);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void ConnectionUI::Show()
{
    HINSTANCE hInst = GetModuleHandleA(NULL);

    if (!CreateMainWindow(hInst))
        return;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}