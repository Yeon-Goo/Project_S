// Fill out your copyright notice in the Description page of Project Settings.

#include "PS_GameMode.h"
#include "PS_GameState.h"
#include "PS_PlayerController.h"
#include "PS_PlayerState.h"
#include "PS_GameInstance.h"
#include "PS_Character.h"
#include "PS_HUD.h"
#include "UObject/ConstructorHelpers.h"


APS_GameMode::APS_GameMode()
{
	GameStateClass = APS_GameState::StaticClass();
	PlayerStateClass = APS_PlayerState::StaticClass();
	PlayerControllerClass = APS_PlayerController::StaticClass();
	HUDClass = APS_HUD::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_Character"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

    CurrentPlayersCount = 0;
    bUseSeamlessTravel = true;
}

void APS_GameMode::BeginPlay()
{
    Super::BeginPlay();

    PS_LOG_S(Log);

    if (UPS_GameInstance* PS_GameInstance = Cast<UPS_GameInstance>(GetGameInstance()))
    {
        CurrentMap = PS_GameInstance->GetMap();
        CurrentStage = PS_GameInstance->GetStage();
    }
    
    APS_GameState* PS_GameState = GetGameState<APS_GameState>();
    if (PS_GameState)
    {
        PS_GameState->SetStage(CurrentMap, CurrentStage);
    }
}

void APS_GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    PS_LOG_S(Log);
    CurrentPlayersCount++;
    UE_LOG(Project_S, Log, TEXT("CurrentPlayersCount : %d"), CurrentPlayersCount);

    // Wait until Players are log-in.
    if (CurrentPlayersCount == 2)
    {
        StartWordSelectionTimer(10);
    }
}

void APS_GameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    CurrentPlayersCount--;
}

void APS_GameMode::TransitionToStage(uint8 MapNumber, uint8 StageNumber)
{
    PS_LOG_S(Log);
    // �� �̸��� �����ϰ� ���� �ε�
    FString MapName = FString::Printf(TEXT("Game/Maps/Level_%d_%d"), MapNumber, StageNumber);

    //GetWorld()->ServerTravel(MapName);
    //GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Maps/Level_0_new?listen")));
    CurrentPlayersCount = 0;
    GetWorld()->ServerTravel(TEXT("/Game/Maps/Level_0_new?listen"), true);
    //GetWorld()->ServerTravel(TEXT("/Game/Maps/Level_0_new"), true, true);

    UE_LOG(Project_S, Log, TEXT("ServerTravel End\n"));
}

void APS_GameMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();

    PS_LOG_S(Log);

    APS_GameState* PS_GameState = GetGameState<APS_GameState>();
    if (PS_GameState)
    {
        // ��� �÷��̾ Ȯ��
        for (APlayerState* PlayerState : PS_GameState->PlayerArray)
        {
            if (PlayerState)
            {
                CurrentPlayersCount++;
                UE_LOG(Project_S, Log, TEXT("Player %s has joined the new map."), *PlayerState->GetPlayerName());
                UE_LOG(Project_S, Log, TEXT("CurrentPlayersCount : %d"), CurrentPlayersCount);
            }
        }
    }

    // 10�� ���� �ܾ� ���� UI ǥ��
    if (CurrentPlayersCount == 2)
    {
        StartWordSelectionTimer(10);
    }
}

void APS_GameMode::StartWordSelectionTimer(int TimeLimit)
{
    PS_LOG_S(Log);
    GetWorldTimerManager().SetTimer(SelectionUITimerHandle, this, &APS_GameMode::OnWordSelectionComplete, TimeLimit, false);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APS_PlayerController* PS_PlayerController = Cast<APS_PlayerController>(It->Get());
        if (PS_PlayerController)
        {
            PS_PlayerController->ShowWordSelectionUI();
        }
    }
}

void APS_GameMode::OnWordSelectionComplete()
{
    PS_LOG_S(Log);
    APS_GameState* PS_GameState = GetGameState<APS_GameState>();
    if (PS_GameState && PS_GameState->AllPlayersSelected())
    {
        StartGameSession(120); // Start 2 min game session
    }
    else
    {
        PS_GameState->DeductLife();

        // Move to Next Level
        CurrentStage++;
        if (CurrentStage > 10)
        {
            CurrentMap++;
            CurrentStage -= 10;
        }

        if (UPS_GameInstance* PS_GameInstance = Cast<UPS_GameInstance>(GetGameInstance()))
        {
            PS_GameInstance->SetMap(CurrentMap);
            PS_GameInstance->SetStage(CurrentStage);

            TransitionToStage(CurrentMap, CurrentStage);
        }
    }
}

void APS_GameMode::StartGameSession(int TimeLimit)
{
    PS_LOG_S(Log);
    GetWorldTimerManager().SetTimer(GameSessionTimerHandle, this, &APS_GameMode::OnGameSessionEnd, TimeLimit, false);
}

void APS_GameMode::OnGameSessionEnd()
{
    PS_LOG_S(Log);
    StartWordSelectionTimer(30);
}