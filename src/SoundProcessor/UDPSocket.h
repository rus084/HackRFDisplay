#pragma once

#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <stdint.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Ws2_32.lib")

class WSASession
{
public:
    WSASession()
    {
        int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (ret != 0)
            throw std::system_error(WSAGetLastError(), std::system_category(), "WSAStartup Failed");
    }
    ~WSASession()
    {
        WSACleanup();
    }
private:
    WSADATA wsaData;
};

class UDPSocket
{
public:
    UDPSocket(const char* ipAddr, unsigned short port)
    {
        int flag_on = 1;              /* socket option flag */
        struct sockaddr_in mc_addr;   /* socket address structure */
        struct ip_mreq mc_req;        /* multicast request structure */

        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            perror("socket() failed");
            exit(1);
        }

        /* set reuse port to on to allow multiple binds per host */
        if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_on,
            sizeof(flag_on))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }

        /* construct a multicast address structure */
        memset(&mc_addr, 0, sizeof(mc_addr));
        mc_addr.sin_family = AF_INET;
        mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        mc_addr.sin_port = htons(port);

        /* bind to multicast address to socket */
        if ((bind(sock, (struct sockaddr*)&mc_addr,
            sizeof(mc_addr))) < 0) {
            perror("bind() failed");
            exit(1);
        }

        /* construct an IGMP join request structure */
        if (inet_pton(AF_INET, ipAddr, &mc_req.imr_multiaddr.s_addr) == 0)
        {
            std::cerr << "Call to InetPton failed." << std::endl;
            exit(1);
        }

        mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

        /* send an ADD MEMBERSHIP message via setsockopt */
        if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (char*)&mc_req, sizeof(mc_req))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
    }

    ~UDPSocket()
    {
        closesocket(sock);
    }

    int ReadSocket(char* buf, int maxLen)
    {
        struct sockaddr_in from_addr; /* packet source */
        int from_len;        /* source addr length */

        from_len = sizeof(from_addr);
        memset(&from_addr, 0, from_len);

        return recvfrom(sock, buf, maxLen, 0,
            (struct sockaddr*)&from_addr, &from_len);
    }

private:
    SOCKET sock;
    WSASession wsa;
};