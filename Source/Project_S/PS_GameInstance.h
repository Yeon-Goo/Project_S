// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_S.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PS_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_S_API UPS_GameInstance : public UGameInstance
{
	GENERATED_BODY()

	UPS_GameInstance();
	
public:
	virtual void Init() override;
	void SetMap(uint8 InMap);
	void SetStage(uint8 InStage);
	void SetLife(uint8 InLife);
	void DeductLife();
	void SetIsGameStart(bool NewBool);

	uint8 GetMap() const;
	uint8 GetStage() const;
	uint8 GetLife() const;
	bool IsGameStart() const;

private:
	uint8 Map;
	uint8 Stage;
	uint8 Life;
	bool bIsGameStart;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
};
