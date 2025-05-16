// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelTcpSocket.h"

void AVoxelTcpSocket::ConnectToGameServer() {
    if (isConnected(connectionIdGameServer))
    {
        //UE_LOG(LogError, Log, TEXT("Log: Can't connect SECOND time. We're already connected!"));
        return;
    }
    FTcpSocketDisconnectDelegate disconnectDelegate;
    disconnectDelegate.BindDynamic(this, &AVoxelTcpSocket::OnDisconnected);
    FTcpSocketConnectDelegate connectDelegate;
    connectDelegate.BindDynamic(this, &AVoxelTcpSocket::OnConnected);
    FTcpSocketReceivedMessageDelegate receivedDelegate;
    receivedDelegate.BindDynamic(this, &AVoxelTcpSocket::OnMessageReceived);
    if (GetNetConnection()) {
        //Server
        UE_LOG(LogTemp, Warning, TEXT("Attempting Server Connect"));
        Connect("10.0.0.75", 6969, disconnectDelegate, connectDelegate, receivedDelegate, connectionIdGameServer);
    }
    else {
        //Client
        UE_LOG(LogTemp, Warning, TEXT("Attempting Client Connect"));
        FString ip = GetWorld()->GetAddressURL();
        UE_LOG(LogTemp, Log, TEXT("Log: IP %s"), *ip);
        Connect("10.0.0.75", 6969, disconnectDelegate, connectDelegate, receivedDelegate, connectionIdGameServer);
    }

}

void AVoxelTcpSocket::OnConnected(int32 ConId) {
    UE_LOG(LogTemp, Log, TEXT("Log: Connected to server."));
}

void AVoxelTcpSocket::OnDisconnected(int32 ConId) {
    UE_LOG(LogTemp, Log, TEXT("Log: OnDisconnected."));
}

void AVoxelTcpSocket::OnMessageReceived(int32 ConId, TArray<uint8>& Message) {
    uint8 payload_type = 0;
    try
    {
        payload_type = Message[0];
        Message.RemoveAt(0);
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Log, TEXT("ERROR: Length of byte array: %s"), e.what());
    }

    //Add Verification on if message is correct format

    FNetPayload netPayload;
    netPayload.payload_type = (EPayloadType)payload_type;
    netPayload.data = Message;

    payloadQueue.Add(netPayload);
}