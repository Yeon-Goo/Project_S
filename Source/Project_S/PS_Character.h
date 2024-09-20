// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PS_Character.generated.h"

//DECLARE_DELEGATE_OneParam(FOnSprint, bool);
UCLASS()
class PROJECT_S_API APS_Character : public ACharacter
{
	GENERATED_BODY()

	// ī�޶�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// �Է� ���� ���ؽ�Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	// �Է� �׼�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterData, meta = (AllowPrivateAccess = "true"))
	class UDataTable* CharacterDataTable;

	struct FPS_CharacterStats* CharacterStats;


public:
	APS_Character();
	// Attack ���� ����
	bool bIsAttacking;

protected:
	virtual void BeginPlay() override;

	void Move(const struct FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintEnd(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void JumpStart(const FInputActionValue& Value);
	void JumpEnd(const FInputActionValue& Value);
	void AttackStart(const FInputActionValue& Value);
	void EndAttack();	

	// Sprint Server functions
	UFUNCTION(Server, Reliable)
	void SprintStart_Server();

	UFUNCTION(Server, Reliable)
	void SprintEnd_Server();

	UFUNCTION(NetMulticast, Reliable)
	void SprintStart_Client();

	UFUNCTION(NetMulticast, Reliable)
	void SprintEnd_Client();

	// Trace�� ����� ����
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 200.0f;

	// ���� ������ �����Ǵ� �ð�
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDuration = 0.5f;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpdateCharacterStats();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE FPS_CharacterStats* GetCharacterStats() const { return CharacterStats; }

};
