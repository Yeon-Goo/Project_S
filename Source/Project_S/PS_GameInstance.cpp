// Fill out your copyright notice in the Description page of Project Settings.


#include "PS_GameInstance.h"

UPS_GameInstance::UPS_GameInstance()
{
    Map = 1;
    Stage = 1;
    Life = 3;
}

void UPS_GameInstance::Init()
{
    IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
    if (OnlineSubsystemInterface)
    {
        SessionInterface = OnlineSubsystemInterface->GetSessionInterface();

        //PS_LOG(Warning, TEXT("Online : %s, this : %s"), *OnlineSubsystemInterface->GetSubsystemName().ToString(), *GetFirstLocalPlayerController()->GetName());

        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GameInstance = World->GetGameInstance())
            {
                FString InstanceName = GameInstance->GetName();
                UE_LOG(Project_S, Log, TEXT("Online : %s, GameInstance : %s"), *OnlineSubsystemInterface->GetSubsystemName().ToString(), *InstanceName);
            }
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Online : %s"), *OnlineSubsystemInterface->GetSubsystemName().ToString()));
        }
        /*
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnJoinSessionComplete);
        }
        */
    }
}

void UPS_GameInstance::SetMap(uint8 InMap)
{
    Map = InMap;
}

void UPS_GameInstance::SetStage(uint8 InStage)
{
    Stage = InStage;
}

void UPS_GameInstance::SetLife(uint8 InLife)
{
    Life = InLife;
}

void UPS_GameInstance::DeductLife()
{
    Life--;
}

uint8 UPS_GameInstance::GetMap() const
{
    return Map;
}

uint8 UPS_GameInstance::GetStage() const
{
    return Stage;
}

uint8 UPS_GameInstance::GetLife() const
{
    return Life;
}

bool UPS_GameInstance::IsGameStart() const
{
    return bIsGameStart;
}

void UPS_GameInstance::SetIsGameStart(bool NewBool)
{
    PS_LOG_S(Log);
    bIsGameStart = NewBool;

    if (bIsGameStart)
    {
        UE_LOG(Project_S, Log, TEXT("bIsGameStart is true\n"));
    }
    else
    {
        UE_LOG(Project_S, Log, TEXT("bIsGameStart is false\n"));
    }
}