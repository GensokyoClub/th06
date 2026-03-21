#include "ConnectionUI.hpp"
#include <sstream>
#include <cstdlib>

#define IDC_EDIT_HOST_IP         1001
#define IDC_EDIT_HOST_PORT       1002
#define IDC_EDIT_LISTEN_PORT     1003
#define IDC_BTN_START_HOST       1004
#define IDC_BTN_START_GUEST      1005
#define IDC_STATIC_LATENCY       1006
#define IDC_EDIT_TARGET_LATENCY  1007
#define IDC_BTN_START_GAME       1008
#define IDC_BTN_START_GAME_LOCAL 1010

#define TIMER_ID_POLL            1
#define TIMER_INTERVAL_MS        15

#define max_delay 10
#define max_delay_s "10"


bool is_ver_matched = true;
ULONGLONG MyGetTickCount()
{
    LARGE_INTEGER l;
    static LARGE_INTEGER f;
    static bool is_inited = false;
    if(!is_inited)
    {
        is_inited = true;
        QueryPerformanceFrequency(&f);
    }
    QueryPerformanceCounter(&l);
    return l.QuadPart*1000/f.QuadPart;
}

ConnectionUI::ConnectionUI(Host& h, Guest& g)
    : m_host(h), m_guest(g)
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

void ConnectionUI::SetDelay(int delay)
{
    m_delay = delay;
    if(m_delay < 0)
    {
        m_delay = 0;
    }
    if(m_delay > max_delay)
    {
        m_delay = max_delay;
    }
    char chs[60];
    sprintf(chs,"%d",m_delay);
    SetWindowTextA(m_editTargetLatency,chs);
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
    char buf[256] = { 0 };
    GetWindowTextA(hEdit, buf, sizeof(buf));
    return std::string(buf);
}

int ConnectionUI::GetEditInt(HWND hEdit)
{
    char buf[64] = { 0 };
    GetWindowTextA(hEdit, buf, sizeof(buf));
    int n=strlen(buf);
    for(int i=0;i<n;i++)
        if(buf[i]>'9' || buf[i]<'0')
        {
            MessageBoxA(NULL,"wrong number","err",MB_OK);
            return -1;
        }
    return atoi(buf);
}

void ConnectionUI::SetText(HWND hWnd, const std::string& s)
{
    SetWindowTextA(hWnd, s.c_str());
}

void ConnectionUI::SetLatencyText(const std::string& s)
{
    SetText(m_staticLatency, s);
}

std::string ConnectionUI::BuildLatencyText(const std::string& ip, int port, ULONGLONG rtt)
{
    std::ostringstream oss;
    oss << ip << ":" << port << "(" << (ULONGLONG)rtt/2 << "ms)";
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
    char title[100];
    sprintf(title,"Launcher [ver=%s]",MULTI_NET_VER_S);

    m_hWnd = CreateWindowA(
        "ConnectionUIClass",
        title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        430, 400,
        NULL, NULL, hInst, this);

    return (m_hWnd != NULL);
}

const char* g_iniPath = ".\\connect_config.ini";

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
}

void ConnectionUI::CreateControls(HWND hWnd)
{
    static char ip[128];
    static char port_host[128];
    static char port_listen[128];
    static char target_delay[128];

    GetPrivateProfileStringA("Connection", "ip", "::1", ip, sizeof(ip), g_iniPath);
    GetPrivateProfileStringA("Connection", "port_host", "3036", port_host, sizeof(port_host), g_iniPath);
    GetPrivateProfileStringA("Connection", "port_listen", "3036", port_listen, sizeof(port_listen), g_iniPath);
    GetPrivateProfileStringA("Connection", "target_delay", "2", target_delay, sizeof(target_delay), g_iniPath);

    CreateWindowA("STATIC", "Host IP:", WS_CHILD | WS_VISIBLE,
        20, 20, 80, 20, hWnd, NULL, NULL, NULL);

    m_editHostIp = CreateWindowA("EDIT", ip,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        110, 20, 270, 24, hWnd, (HMENU)IDC_EDIT_HOST_IP, NULL, NULL);

    CreateWindowA("STATIC", "Host Port:", WS_CHILD | WS_VISIBLE,
        20, 60, 80, 20, hWnd, NULL, NULL, NULL);

    m_editHostPort = CreateWindowA("EDIT", port_host,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
        110, 60, 100, 24, hWnd, (HMENU)IDC_EDIT_HOST_PORT, NULL, NULL);

    CreateWindowA("STATIC", "Listen Port:", WS_CHILD | WS_VISIBLE,
        20, 100, 80, 20, hWnd, NULL, NULL, NULL);

        
    m_editListenPort = CreateWindowA("EDIT", port_listen,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
        110, 100, 100, 24, hWnd, (HMENU)IDC_EDIT_LISTEN_PORT, NULL, NULL);

    m_btnHost = CreateWindowA("BUTTON", "as host",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 150, 160, 32, hWnd, (HMENU)IDC_BTN_START_HOST, NULL, NULL);

    m_btnGuest = CreateWindowA("BUTTON", "as guest",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 150, 160, 32, hWnd, (HMENU)IDC_BTN_START_GUEST, NULL, NULL);

    CreateWindowA("STATIC", "cur state:", WS_CHILD | WS_VISIBLE,
        20, 210, 80, 20, hWnd, NULL, NULL, NULL);

    m_staticLatency = CreateWindowA("STATIC", "no connection",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        110, 210, 270, 24, hWnd, (HMENU)IDC_STATIC_LATENCY, NULL, NULL);

        
    CreateWindowA("STATIC", "target delay:", WS_CHILD | WS_VISIBLE,
        20, 250, 120, 20, hWnd, NULL, NULL, NULL);

    m_editTargetLatency = CreateWindowA("EDIT", target_delay,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
        110, 250, 100, 24, hWnd, (HMENU)IDC_EDIT_TARGET_LATENCY, NULL, NULL);

    m_btnStartGame = CreateWindowA("BUTTON", "Start Game",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED,
        20, 300, 160, 32, hWnd, (HMENU)IDC_BTN_START_GAME, NULL, NULL);

    m_btnStartGameLocal = CreateWindowA("BUTTON", "Start Game(local)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 300, 160, 32, hWnd, (HMENU)IDC_BTN_START_GAME_LOCAL, NULL, NULL);

    m_delay = atoi(target_delay);
    if(m_delay<0)
    {
        m_delay=0;
        SetWindowTextA(m_editTargetLatency,"0");
    }
    if(m_delay>max_delay)
    {
        m_delay=max_delay;
        SetWindowTextA(m_editTargetLatency,max_delay_s);
    };
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

bool ConnectionUI::TryStartGuest(const std::string& hostIp, int hostPort, int listenPort)
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

void ConnectionUI::SendPingAsHost(Control ctrl)
{
    CtrlPack cp;
    cp.ctrl_type = ctrl;
    cp.init_setting.delay = GetDelay();
    cp.init_setting.ver = MULTI_NET_VER;

    Pack p;
    p.ctrl = cp;
    p.type = PACK_PING;
    p.seq = m_seq++;
    p.sendTick = MyGetTickCount();
    p.echoTick = 0;

    m_host.SendPack(p);
}

void ConnectionUI::SendPingAsGuest(Control ctrl)
{
    CtrlPack cp;
    cp.ctrl_type = ctrl;
    cp.init_setting.delay = GetDelay();
    cp.init_setting.ver = MULTI_NET_VER;

    Pack p;
    p.ctrl = cp;
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
            SendPingAsHost(Ctrl_Set_InitSetting);
        }
        else if (m_isGuest)
        {
            SendPingAsGuest(Ctrl_Set_InitSetting);
        }

        m_lastPeriodicPingTick = now;
    }
}

void ConnectionUI::OnClickHost()
{
    is_ver_matched = true;
    int listenPort = GetEditInt(m_editListenPort);
    if(listenPort==-1)
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
    is_ver_matched = true;
    std::string hostIp = GetEditText(m_editHostIp);
    int hostPort = GetEditInt(m_editHostPort);
    int listenPort = GetEditInt(m_editListenPort);
    EnableWindow(m_editTargetLatency, FALSE);

    if(listenPort==-1 || hostPort==-1)
        return;

    if (!TryStartGuest(hostIp, hostPort, listenPort))
    {
        MessageBoxA(m_hWnd, "fail to start as guest", "err", MB_OK | MB_ICONERROR);
        return;
    }

    EnterGuestWaitingState();

    // ping
    SendPingAsGuest(Ctrl_Set_InitSetting);
}

void ConnectionUI::OnClickStartGame()
{
    if (!m_connected)
        return;
    m_startGame = true;
    if(this->IsHost())
        SendPingAsHost(Ctrl_Start_Game);
    else
        SendPingAsGuest(Ctrl_Start_Game);
    return;
    
    // DestroyWindow(m_hWnd);
}

void ConnectionUI::ProcessHostNetwork()
{
    if(!is_ver_matched)
        return;
    while (true)
    {
        Pack p;
        bool hasData = false;

        if (!m_host.PollReceive(p, hasData))
            break;

        if (!hasData)
            break;

        if(p.ctrl.ctrl_type == Ctrl_Set_InitSetting && p.ctrl.init_setting.ver != MULTI_NET_VER)
            is_ver_matched = false;

        // host pong
        if (p.type == PACK_PING)
        {
            
            Pack reply;
            reply.type = PACK_PONG;
            reply.seq = p.seq;
            reply.sendTick = p.sendTick;           // cal RTT
            reply.echoTick = MyGetTickCount();
            reply.ctrl = p.ctrl;
            reply.ctrl.init_setting.ver = MULTI_NET_VER;
            m_host.SendPack(reply);
            if (!m_connected)
                EnterConnectedState();
            if(p.ctrl.ctrl_type==Ctrl_Start_Game)
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
            if(p.ctrl.ctrl_type==Ctrl_Start_Game)
            {
                DestroyWindow(m_hWnd);
            }
        }

        if(!is_ver_matched)
        {
            MessageBoxA(NULL,"not matched guest/host version","warning",MB_OK | MB_ICONWARNING);
            m_host.Reset();
            m_isHost = false;
            m_isGuest = false;
            m_connected = false;
            m_lastPeriodicPingTick = 0;

            EnableWindow(m_btnGuest, TRUE);
            SetText(m_btnHost, "as host");
            SetLatencyText("no connection");
            EnableWindow(m_btnStartGame, FALSE);
            return;
        }
    }
}

void ConnectionUI::ProcessGuestNetwork()
{
    bool gotAnyData = false;
    if(!is_ver_matched)
        return;
    while (true)
    {
        Pack p;
        bool hasData = false;

        if (!m_guest.PollReceive(p, hasData))
            break;

        if (!hasData)
            break;

        gotAnyData = true;
        
        if(p.ctrl.ctrl_type == Ctrl_Set_InitSetting && p.ctrl.init_setting.ver != MULTI_NET_VER)
            is_ver_matched = false;

        // guest rcv ping
        if (p.type == PACK_PING)
        {
            Pack reply;
            reply.type = PACK_PONG;
            reply.seq = p.seq;
            reply.sendTick = p.sendTick;
            reply.echoTick = MyGetTickCount();
            reply.ctrl = p.ctrl;
            reply.ctrl.init_setting.ver = MULTI_NET_VER;
            m_guest.SendPack(reply);

            if (!m_connected)
                EnterConnectedState();

            if(p.ctrl.ctrl_type==Ctrl_Start_Game)
            {
                m_startGame = true;
                DestroyWindow(m_hWnd);
            }else if(p.ctrl.ctrl_type==Ctrl_Set_InitSetting){
                SetDelay(p.ctrl.init_setting.delay);
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

            if(p.ctrl.ctrl_type==Ctrl_Start_Game)
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
    }else if(!is_ver_matched)
    {
        MessageBoxA(NULL,"not matched guest/host version","warning",MB_OK | MB_ICONWARNING);
        m_guest.Reset();
        m_isHost = false;
        m_isGuest = false;
        m_connected = false;
        m_lastPeriodicPingTick = 0;
        m_guestWaitStartTick = 0;

        EnableWindow(m_btnHost, TRUE);
        SetText(m_btnGuest, "as guest");
        SetLatencyText("no connection");
        EnableWindow(m_btnStartGame, FALSE);
        return;
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
    ConnectionUI* pThis = NULL;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTA* pcs = (CREATESTRUCTA*)lParam;
        pThis = (ConnectionUI*)pcs->lpCreateParams;
        SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hWnd;
    }
    else
    {
        pThis = (ConnectionUI*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
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

    case WM_COMMAND:
    {
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
                if (HIWORD(wParam) == EN_CHANGE) {
                    char buf[32];
                    GetWindowTextA(m_editTargetLatency, buf, 16);
                    m_delay = atoi(buf);
                    if(m_delay<0)
                    {
                        m_delay=0;
                        SetWindowTextA(m_editTargetLatency,"0");
                    }
                    if(m_delay>max_delay)
                    {
                        m_delay=max_delay;
                        SetWindowTextA(m_editTargetLatency,max_delay_s);
                        SendMessage((HWND)lParam, EM_SETSEL, 2, 2);
                    }
                    if(IsHost())
                        TryPeriodicPing();
                }
                break;
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