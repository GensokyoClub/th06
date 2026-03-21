#pragma once
#define MULTI_NET_VER 3910
#define MULTI_NET_VER_S "3.9.1"

#include <Ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>

#include <cstring>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

void PrintError(char *msg);
void PrintError(char *msg, int err);
void PrintLog(char *msg);

bool MyInetPton(int family, const char *src, void *dst);
const char *MyInetNtop(int family, const void *src, char *dst, size_t size);

#pragma pack(push, 1)

template <int N_Bits> struct Bits
{
    unsigned char data[N_Bits / 8];
    inline int Read(int n_bit) const
    {
        return (data[n_bit >> 3] >> (n_bit & 7)) & 1u;
    }
    inline void Write(int n_bit, int bit)
    {
        unsigned char &c = data[n_bit >> 3];
        const unsigned char mask = static_cast<unsigned char>(1u << (n_bit & 7));
        c = static_cast<unsigned char>((c & ~mask) | ((-(bit & 1)) & mask));
    }

    inline void Clear()
    {
        std::memset(data, 0, sizeof(data));
    }
    inline Bits operator|(const Bits &b2) const
    {
        Bits r;
        for (int i = 0; i < N_Bits / 8; ++i)
            r.data[i] = static_cast<unsigned char>(data[i] | b2.data[i]);
        return r;
    }
    inline Bits &operator|=(const Bits &b2)
    {
        for (int i = 0; i < N_Bits / 8; ++i)
            data[i] = static_cast<unsigned char>(data[i] | b2.data[i]);
        return *this;
    }
};

inline void ReadFromInt(Bits<8> &b, unsigned char i)
{
    b.data[0] = i;
}

inline void WriteToInt(const Bits<8> &b, unsigned char &i)
{
    i = b.data[0];
}

inline void ReadFromInt(Bits<16> &b, unsigned short i)
{
    b.data[0] = (unsigned char)(i & 0xFF);
    b.data[1] = (unsigned char)((i >> 8) & 0xFF);
}

inline void WriteToInt(const Bits<16> &b, unsigned short &i)
{
    i = (unsigned short)(b.data[0]) | (unsigned short)(b.data[1] << 8);
}

inline void ReadFromInt(Bits<32> &b, unsigned int i)
{
    b.data[0] = (unsigned char)(i & 0xFF);
    b.data[1] = (unsigned char)((i >> 8) & 0xFF);
    b.data[2] = (unsigned char)((i >> 16) & 0xFF);
    b.data[3] = (unsigned char)((i >> 24) & 0xFF);
}

inline void WriteToInt(const Bits<32> &b, unsigned int &i)
{
    i = (unsigned int)(b.data[0]) | ((unsigned int)(b.data[1]) << 8) | ((unsigned int)(b.data[2]) << 16) |
        ((unsigned int)(b.data[3]) << 24);
}

enum Control
{
    Ctrl_No_Ctrl,
    Ctrl_Start_Game,
    Ctrl_Key,
    Ctrl_Set_InitSetting,
    Ctrl_Try_Resync
};

enum InGameCtrlType
{
    Quick_Quit,
    Quick_Restart,
    Inf_Life,
    Inf_Bomb,
    Inf_Power,
    Add_Delay,
    Dec_Delay,
    Insane_Mode,
    IGC_NONE
};

#define KeyPackFrameNum 15
struct CtrlPack
{
    int frame;
    Control ctrl_type;

    union {
        Bits<16> keys[KeyPackFrameNum];
        struct
        {
            int delay;
            int ver;
        } init_setting;
        struct
        {
            int frame_to_re_sync;
        } resync_setting;
    };
    InGameCtrlType igc_type[KeyPackFrameNum];
    unsigned short rng_seed[KeyPackFrameNum];

    CtrlPack() : ctrl_type(Ctrl_No_Ctrl), frame(0)
    {
        memset(keys, 0, sizeof(keys));
    }
};

struct Pack
{
    int type; // 1=HELLO, 2=PING, 3=PONG, 4=usual trans
    unsigned int seq;
    ULONGLONG sendTick;
    ULONGLONG echoTick;
    CtrlPack ctrl;

    Pack() : ctrl()
    {
        type = 0;
        seq = 0;
        sendTick = 0;
        echoTick = 0;
    }
};
#pragma pack(pop)

class ConnectionBase
{
  protected:
    SOCKET m_socket;
    int m_family;

  protected:
    static bool s_winsockInited;
    static int s_refCount;

  public:
    ConnectionBase();
    virtual ~ConnectionBase();
    void Reset();

  protected:
    bool InitWinsock();
    void CleanupWinsock();

    bool CreateUdpSocket(int family);
    bool SetNonBlocking();
    bool BindSocket(const std::string &bindIp, int port, int family);

    bool SendPackTo(const Pack &pack, const std::string &ip, int port);
    bool ReceiveOnePack(Pack &outPack, std::string &fromIp, int &fromPort, bool &hasData);

    void CloseSocket();

    bool IpPortToSockAddr(const std::string &ip, int port, sockaddr_storage &addr, int &addrLen, int family);

    bool SockAddrToIpPort(const sockaddr *addr, int addrLen, std::string &ip, int &port);
};

class Host : public ConnectionBase
{
  private:
    std::string m_hostIp;
    int m_hostPort;

    std::string m_guestIp;
    int m_guestPort;
    std::string m_lastBindIp;
    int m_lastBindPort;
    int m_lastFamily;

  public:
    Host();
    virtual ~Host();

  public:
    bool Start(const std::string &bindIp, int port, int family = AF_INET6);
    bool PollReceive(Pack &outPack, bool &hasData);
    bool SendPack(const Pack &pack);

    bool IsHost() const;
    bool IsGuest() const;

    std::string GetHostIp() const;
    int GetHostPort() const;

    void SetGuestIp(std::string ip);
    void SetGuestPort(int port);

    std::string GetGuestIp() const;
    int GetGuestPort() const;
    void Reset();
    void Reconnect();
};

class Guest : public ConnectionBase
{
  private:
    std::string m_hostIp;
    int m_hostPort;

    int m_localPort;

    std::string m_lastHostIp;
    int m_lastHostPort;
    int m_lastLocalPort;
    int m_lastFamily;

  public:
    Guest();
    virtual ~Guest();

  public:
    bool Start(const std::string &hostIp, int hostPort, int localPort, int family);
    bool PollReceive(Pack &outPack, bool &hasData);
    bool SendPack(const Pack &pack);

    bool IsHost() const;
    bool IsGuest() const;

    std::string GetHostIp() const;
    int GetHostPort() const;
    int GetLocalPort() const;
    void Reset();
    void Reconnect();
};