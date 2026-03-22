#pragma once


#include <string>
#include "connection.hpp"
#include <windows.h>

class ConnectionUI
{
public:
    ConnectionUI(Host& h, Guest& g);
    ~ConnectionUI();

public:
    void Show();
    int GetDelay();
    void SetDelay(int delay);

    bool IsHost() const;
    bool IsGuest() const;
    bool IsConnected();
    bool IsGameStarted();
private:
    enum
    {
        PACK_HELLO = 1,
        PACK_PING  = 2,
        PACK_PONG  = 3
    };

private:

    Host& m_host;
    Guest& m_guest;

    bool m_isHost;
    bool m_isGuest;
    int m_delay;

    bool m_connected;
    bool m_startGame;

    HWND m_hWnd;

    HWND m_editHostIp;
    HWND m_editHostPort;
    HWND m_editListenPort;
    HWND m_btnHost;
    HWND m_btnGuest;
    HWND m_staticLatency;
    HWND m_editTargetLatency;
    HWND m_btnStartGame;
    HWND m_btnStartGameLocal;
    HWND m_checkBoxIsHost1P;

    ULONGLONG m_guestWaitStartTick;
    ULONGLONG m_lastPeriodicPingTick;

    unsigned int m_seq;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool CreateMainWindow(HINSTANCE hInst);
    void SaveControls();
    void CreateControls(HWND hWnd);

    void OnClickHost();
    void OnClickGuest();
    void OnClickStartGame();
    void OnTimer();

    void ProcessHostNetwork();
    void ProcessGuestNetwork();

    void TryPeriodicPing();
    void SendPingAsHost(Control ctrl);
    void SendPingAsGuest(Control ctrl);

    void EnterHostWaitingState();
    void EnterGuestWaitingState();
    void EnterConnectedState();
    void ResetGuestButtonAfterTimeout();

    std::string GetEditText(HWND hEdit);
    int GetEditInt(HWND hEdit);
    void SetText(HWND hWnd, const std::string& s);
    void SetLatencyText(const std::string& s);

    std::string BuildLatencyText(const std::string& ip, int port, ULONGLONG rtt);

    bool TryStartHost(int listenPort);
    bool TryStartGuest(const std::string& hostIp, int hostPort, int listenPort);
    
};