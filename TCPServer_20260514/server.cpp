#include "stdafx.h"

int main()
{
    int PlayerX = 0;
    int PlayerY = 0;

    WSAData WsaData;
    WSAStartup(MAKEWORD(2, 2), &WsaData);

    SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN ListenAddr;
    ZeroMemory(&ListenAddr, sizeof(ListenAddr));
    ListenAddr.sin_family = AF_INET;
    ListenAddr.sin_addr.s_addr = INADDR_ANY;
    ListenAddr.sin_port = htons(SERVER_PORT);

    bind(ListenSocket, (SOCKADDR*)&ListenAddr, sizeof(ListenAddr));
    listen(ListenSocket, 1);

    printf("Server started. Port 31000. Waiting for client...\n");

    while (true)
    {
        // 클라이언트 접속 대기 (블로킹)
        SOCKADDR_IN ClientAddr;
        ZeroMemory(&ClientAddr, sizeof(ClientAddr));
        int ClientAddrLen = sizeof(ClientAddr);
        SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientAddr, &ClientAddrLen);

        char ClientIP[64] = { 0, };
        inet_ntop(AF_INET, &ClientAddr.sin_addr, ClientIP, sizeof(ClientIP));
        printf("Client connected: %s\n", ClientIP);

        PacketHeader SendHeader;
        PositionData SendPos;
        int WantSend = 0;
        int TotalSent = 0;
        int Sent = 0;
        int Running = 1;

        // -------------------------------------------------------
        // 이동 패킷 수신 루프
        // -------------------------------------------------------
        while (Running)
        {
            // 헤더 수신 (블로킹)
            PacketHeader RecvHeader;
            int RecvBytes = recv(ClientSocket, (char*)&RecvHeader, sizeof(RecvHeader), MSG_WAITALL);
            if (RecvBytes <= 0)
            {
                printf("Client disconnected.\n");
                break;
            }

            RecvHeader.Size = ntohs(RecvHeader.Size);
            RecvHeader.Code = ntohs(RecvHeader.Code);

            // 데이터 수신 (블로킹)
            MoveData Move;
            recv(ClientSocket, (char*)&Move, RecvHeader.Size, MSG_WAITALL);

            // 이동 처리
            if ((PacketType)RecvHeader.Code == PacketType::Move)
            {
                int NewX = PlayerX;
                int NewY = PlayerY;

                switch (Move.Dir)
                {
                case 'W':
                case 'w':
                    NewY--;
                    break;
                case 'S':
                case 's':
                    NewY++;
                    break;
                case 'A':
                case 'a':
                    NewX--;
                    break;
                case 'D':
                case 'd':
                    NewX++;
                    break;
                }

                PlayerX = NewX;
                PlayerY = NewY;

                printf("Player [%c] -> (%d, %d)\n", Move.Dir, PlayerX, PlayerY);

                // 갱신된 위치 전송 (헤더)
                SendHeader.Size = htons((unsigned short)sizeof(PositionData));
                SendHeader.Code = htons((unsigned short)PacketType::Position);

                SendPos.X = htonl((u_long)PlayerX);
                SendPos.Y = htonl((u_long)PlayerY);

                WantSend = sizeof(SendHeader);
                TotalSent = 0;

                do
                {
                    Sent = send(ClientSocket, (char*)&SendHeader + TotalSent, WantSend - TotalSent, 0);
                    if (Sent <= 0)
                    {
                        printf("send error\n");
                        Running = 0;
                        break;
                    }
                    TotalSent += Sent;
                } while (TotalSent < WantSend);

                if (!Running)
                {
                    break;
                }

                // 갱신된 위치 전송 (데이터)
                WantSend = sizeof(SendPos);
                TotalSent = 0;

                do
                {
                    Sent = send(ClientSocket, (char*)&SendPos + TotalSent, WantSend - TotalSent, 0);
                    if (Sent <= 0)
                    {
                        printf("send error\n");
                        Running = 0;
                        break;
                    }
                    TotalSent += Sent;
                } while (TotalSent < WantSend);
            }
        }

        shutdown(ClientSocket, SD_BOTH);
        closesocket(ClientSocket);
    }


    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}