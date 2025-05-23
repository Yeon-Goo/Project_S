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
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GameInstance = World->GetGameInstance())
            {
                UE_LOG(Project_S, Log, TEXT("Online : %s, GameInstance : %s"), *OnlineSubsystem->GetSubsystemName().ToString(), *GameInstance->GetName());

                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Online : %s, GameInstance : %s"), *OnlineSubsystem->GetSubsystemName().ToString(), *GameInstance->GetName()));
                }
            }
        }

        if (SessionInterface.IsValid())
        {
            // Setup Delegates
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPS_GameInstance::OnJoinSessionComplete);

            UE_LOG(Project_S, Log, TEXT("GameInstance : %s, Setup Delegates Complete"), *this->GetName());
        }
        else
        {
            PS_LOG_S(Log);
            UE_LOG(Project_S, Warning, TEXT("SessionInterface is Not Valid!!"));
        }
    }

    bIsJoining = false;
}

void UPS_GameInstance::CreateSession()
{
    PS_LOG_S(Log);
    UE_LOG(Project_S, Log, TEXT("Create Session"));
    if (SessionInterface.IsValid())
    {
        // Set the Handle
        SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSesionCompleteDelegate);

        // ���� Session ����
        const auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); // 
        if (ExistingSession != nullptr)
        {
            UE_LOG(Project_S, Log, TEXT("There is Existing Session.. Destroying.."));
            SessionInterface->DestroySession(NAME_GameSession);
        }
        else
        {
            UE_LOG(Project_S, Log, TEXT("There is no Existing Session.."));
        }

        TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
        SessionSettings->bIsLANMatch = false;			// Lan�� ���� ����
        SessionSettings->NumPublicConnections = 2;		// ���� ���� �ο�
        SessionSettings->bAllowJoinInProgress = false;	// ������ ���� �� �������� ����
        SessionSettings->bAllowJoinViaPresence = true;	// ???
        SessionSettings->bShouldAdvertise = true;		// Steam�� ���� ������ �˸�(���� ��ȸ ����)
        SessionSettings->bUsesPresence = true;			// ???
        //SessionSettings->bAllowInvites = true;
        //SessionSettings->bAllowJoinViaPresence = true;
        //SessionSettings->bAllowJoinViaPresenceFriendsOnly = true;
        SessionSettings->bUseLobbiesIfAvailable = true; // Lobby ��� ����
        SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); // ������ MatchType�� ��ο��� ����, �¶��μ��񽺿� ���� ���� ���� ȫ�� �ɼ����� ����

        // GameInstance���� �����ϱ� ����(PlayerController�� ���� �� �ְų� ���� �ִٸ� �ᵵ��)
        const ULocalPlayer* Localplayer = GetWorld()->GetFirstLocalPlayerFromController();
        SessionInterface->CreateSession(*Localplayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
        UE_LOG(Project_S, Log, TEXT("Create Session Complete"));
    }
    else
    {
        UE_LOG(Project_S, Log, TEXT("Create Session Error"));
    }
}

void UPS_GameInstance::DestroySession()
{
    UE_LOG(Project_S, Log, TEXT("Destroy Session"));
    if (SessionInterface.IsValid())
    {
        // Set the Handle
        FDelegateHandle DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

        const FName SessionName = NAME_GameSession;

        if (SessionInterface->DestroySession(SessionName))
        {
            UE_LOG(Project_S, Log, TEXT("Requested destroy for session [%s]"), *SessionName.ToString());
            UE_LOG(Project_S, Log, TEXT("Destroy Session Complete"));
        }
        else
        {
            UE_LOG(Project_S, Log, TEXT("Failed to start DestroySession for [%s]"), *SessionName.ToString());
            // ���� ��� Delegate �� �������൵ �����ϴ�:
            //SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
        }
    }
}

void UPS_GameInstance::FindSession()
{
    UE_LOG(Project_S, Log, TEXT("Find Session Start"));

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
    {
        UE_LOG(Project_S, Error, TEXT("LocalPlayer or UniqueNetId is not valid."));
        return;
    }

    if (SessionInterface.IsValid())
    {
        // Find Session Complete Delegate ���
        SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

        // Find Game Session
        SessionSearch = MakeShareable(new FOnlineSessionSearch());
        UE_LOG(Project_S, Log, TEXT("SessionSearch created: %p"), SessionSearch.Get());
        SessionSearch->MaxSearchResults = 10;	// �˻� ����� ������ ���� �� �ִ밪
        SessionSearch->bIsLanQuery = false;			// LAN ��� ����
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); // ã�� ���� ������ ����� �����Ѵ�

        //const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
        UE_LOG(Project_S, Log, TEXT("Find Session Complete"));
    }
    else
    {
        UE_LOG(Project_S, Log, TEXT("Find Session Error"));
    }
}

void UPS_GameInstance::JoinSession(int Index)
{
    UE_LOG(Project_S, Log, TEXT("Join Session Start"));
    UE_LOG(Project_S, Log, TEXT("Index: %d"), Index);

    if (SessionInterface.IsValid())
    {
        // Set the Handle
        SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

        // idx: ���ϴ� Session�� index
        //int idx = 0;
        const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
        auto SessionName = FName(SessionSearch->SearchResults[Index].Session.OwningUserName);
        auto Result = SessionSearch->SearchResults[Index];
        SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), SessionName, Result);
        UE_LOG(Project_S, Log, TEXT("Join Session Complete"));
    }
    else
    {
        UE_LOG(Project_S, Log, TEXT("Join Session Error"));
    }
}

void UPS_GameInstance::OnCreateSessionComplete(FName SesionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        // ���� ���� ����
        UE_LOG(Project_S, Log, TEXT("Create Session Complete Successfully"));
        bool IsSuccessful = SessionInterface->StartSession(NAME_GameSession);

        if (IsSuccessful)
        {
            UE_LOG(Project_S, Log, TEXT("Start Session Complete"));
            //ClientTravel(TEXT("/Game/Maps/Level_MainMenu"), ETravelType::TRAVEL_Absolute);
        }
        else
        {
            UE_LOG(Project_S, Log, TEXT("Start Session Fail"));
        }
    }
    else
    {
        // ���� ���� ����
    }
}

void UPS_GameInstance::OnDestroySessionComplete(FName SesionName, bool bWasSuccessful)
{
    UE_LOG(Project_S, Log, TEXT("Destroy Session Complete Successfully"));
    if (bWasSuccessful)
    {
        // ���� ���� ����
        BlueprintDestroySessionsCompleteDelegate.Broadcast();
        UE_LOG(Project_S, Log, TEXT("Destroy Session Complete"));
    }
    else
    {
        // ���� ���� ����
        UE_LOG(Project_S, Log, TEXT("Start Session Fail"));
    }
}

void UPS_GameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(Project_S, Log, TEXT("GameInstance : %s, Find Sessions Complete Successfully"), *this->GetName());
        UE_LOG(Project_S, Log, TEXT("OnFindSessionsComplete called. SessionSearch pointer: %p"), SessionSearch.Get());
        // ����
        if (SessionSearch.IsValid())
        {
            int cnt = 1;
            TArray<FBlueprintSessionResult> BlueprintSessionResults;
            for (const auto& Result : SessionSearch->SearchResults)
            {
                if (Result.IsValid())
                {
                    UE_LOG(Project_S, Warning, TEXT("cnt = %d"), cnt);
                    cnt++;
                    FBlueprintSessionResult BlueprintSessionResult;
                    BlueprintSessionResult.OnlineResult = Result;
                    BlueprintSessionResults.Add(BlueprintSessionResult);
                }
                else
                {
                    UE_LOG(Project_S, Warning, TEXT("SearchResult not Valid"));
                }
            }

            UE_LOG(Project_S, Warning, TEXT("BlueprintSessionResults.size() = %d"), BlueprintSessionResults.Num());
            BlueprintFindSessionsCompleteDelegate.Broadcast(BlueprintSessionResults);
        }
        else
        {
            UE_LOG(Project_S, Log, TEXT("SessionSearch is not Valid"));
        }
    }
    else
    {
        // ���� ���� ����
        UE_LOG(Project_S, Log, TEXT("Find Sessions Complete Failure"));
    }
}

void UPS_GameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(Project_S, Log, TEXT("GameInstance : %s, Find Sessions Complete Successfully"), *this->GetName());
    UE_LOG(Project_S, Log, TEXT("OnFindSessionsComplete called. SessionSearch pointer: %p"), SessionSearch.Get());

    switch (Result)
    {
    /** The join worked as expected */
    case EOnJoinSessionCompleteResult::Success:
        if (SessionInterface.IsValid())
        {
            APlayerController* const PlayerController = GetFirstLocalPlayerController();
            FString TravelURL;

            if (PlayerController && SessionInterface->GetResolvedConnectString(SessionName, TravelURL))
            {
                PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
            }
            else
            {
                UE_LOG(Project_S, Log, TEXT("Failed to resolve connect string for session %s"), *SessionName.ToString());
                BlueprintJoinSessionsCompleteDelegate.Broadcast();
            }
        }

        break;
    /** There are no open slots to join */
    case EOnJoinSessionCompleteResult::SessionIsFull:
        UE_LOG(Project_S, Log, TEXT("Cannot join session %s: Session is full"), *SessionName.ToString());
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("���� ���� á���ϴ�."));
        }
        BlueprintJoinSessionsCompleteDelegate.Broadcast();
        break;
    /** The session couldn't be found on the service */
    case EOnJoinSessionCompleteResult::SessionDoesNotExist:
        UE_LOG(Project_S, Log, TEXT("Cannot join session %s: Session does not exist"), *SessionName.ToString());
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("������ ã�� �� �����ϴ�."));
        }
        BlueprintJoinSessionsCompleteDelegate.Broadcast();
        break;
    /** There was an error getting the session server's address */
    case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
        UE_LOG(Project_S, Log, TEXT("Cannot join session %s: Could not retrieve server address"), *SessionName.ToString());
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("���� �ּҸ� ������ �� �����ϴ�."));
        }
        BlueprintJoinSessionsCompleteDelegate.Broadcast();
        break;
    /** The user attempting to join is already a member of the session */
    case EOnJoinSessionCompleteResult::AlreadyInSession:
        UE_LOG(Project_S, Log, TEXT("Player is already in session %s"), *SessionName.ToString());
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("�̹� �ش� ���ǿ� ���ӵǾ� �ֽ��ϴ�."));
        }
        BlueprintJoinSessionsCompleteDelegate.Broadcast();
        break;
    /** An error not covered above occurred */
    case EOnJoinSessionCompleteResult::UnknownError:
        UE_LOG(Project_S, Log, TEXT("Unknown error joining session %s, Result=%d"), *SessionName.ToString(), static_cast<int32>(Result));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("�� �� ���� ������ ���ӿ� �����߽��ϴ�."));
        }
        BlueprintJoinSessionsCompleteDelegate.Broadcast();
        break;
    }

    bIsJoining = false;
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