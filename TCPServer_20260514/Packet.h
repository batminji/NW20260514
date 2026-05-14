#pragma once

#ifndef __PACKET_H__
#define __PACKET_H__


enum class PacketType
{
    Move = 0,   // 클라이언트 -> 서버: 이동 방향
    Position,   // 서버 -> 클라이언트: 플레이어 위치
    Max
};

#pragma pack(push, 1)

// 공통 헤더: Size(데이터 크기) + Code(패킷 종류)
struct PacketHeader
{
    unsigned short Size;
    unsigned short Code;
};

// 이동 방향 데이터 (클라이언트 -> 서버)
struct MoveData
{
    char Dir;   // 'W', 'A', 'S', 'D'
};

// 위치 데이터 (서버 -> 클라이언트)
struct PositionData
{
    int X;
    int Y;
};

#pragma pack(pop)

#endif // __PACKET_H__
