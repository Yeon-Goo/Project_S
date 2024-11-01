// Fill out your copyright notice in the Description page of Project Settings.


#include "PS_Enemy.h"

// Core Engine Components
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// DataTable and AnimInstance
#include "Engine/DataTable.h"


#include "AIController.h"
#include "NavigationSystem.h"
#include "PS_Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/SphereComponent.h"

// Sets default values
APS_Enemy::APS_Enemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Controller�� ȸ���� Character�� ȸ���� ������� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	// Setup PawnSensing Component
	PawnSense = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSense"));
	PawnSense->SensingInterval = 0.8f;
	PawnSense->SetPeripheralVisionAngle(45.0f);
	PawnSense->SightRadius = 1500.0f;
	PawnSense->HearingThreshold = 400.0f;
	PawnSense->LOSHearingThreshold = 800.0f;

	// Setup Collision
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(100);
	Collision->SetupAttachment(RootComponent);

	// CapsuleComponent ����
	GetCapsuleComponent()->InitCapsuleSize(50.0f, 90.0f);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	// Mesh ����
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/Characters/Skeletons/Skeleton_Minion/Skeleton_Minion"));
	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}

	// CharacterMovement ����
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->GravityScale = 4.0f;
}

// Called when the game starts or when spawned
void APS_Enemy::BeginPlay()
{
	Super::BeginPlay();

	SetNextPatrolLocation();
	
	//UpdateEnemyStats();
}

// Called every frame
void APS_Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() != ROLE_Authority) return;

	if (GetMovementComponent()->GetMaxSpeed() == ChaseSpeed) return;

	if ((GetActorLocation() - PatrolLocation).Size() < 500.0f)
	{
		SetNextPatrolLocation();
	}

}

void APS_Enemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetLocalRole() != ROLE_Authority) return;

	// Setup Delegates
	OnActorBeginOverlap.AddDynamic(this, &APS_Enemy::OnBeginOverlap);
	GetPawnSense()->OnSeePawn.AddDynamic(this, &APS_Enemy::OnPawnDetected);
}

void APS_Enemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called to bind functionality to input
void APS_Enemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APS_Enemy::UpdateEnemyStats()
{
	/*
	if (EnemyDataTable)
	{
		TArray<FPS_CharacterStats*> CharacterStatsRows;
		EnemyDataTable->GetAllRows<FPS_CharacterStats>(TEXT("PS_Character"), CharacterStatsRows);

		if (CharacterStatsRows.Num() > 0)
		{
			//const auto NewCharacterLevel = FMath::Clamp(CharacterLevel, 1, CharacterStatsRows.Num());
			//CharacterStats = CharacterStatsRows[NewCharacterLevel - 1];
			CharacterStats = CharacterStatsRows[0];

			GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->WalkSpeed;
		}
	}
	*/
}

void APS_Enemy::SetNextPatrolLocation()
{
	if (GetLocalRole() != ROLE_Authority) return;

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	const auto LocationFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, GetActorLocation(), PatrolLocation, PatrolRadius);
	if (LocationFound)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), PatrolLocation);
	}

}

void APS_Enemy::Chase(APawn* Pawn)
{
	if (GetLocalRole() != ROLE_Authority) return;

	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), Pawn);

	DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), 25.0f, 12, FColor::Red, true, 10.0f, 0, 2.0f);
}

void APS_Enemy::OnPawnDetected(APawn* Pawn)
{
	if (!Pawn->IsA<APS_Character>()) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Character detected!"));

	if (GetCharacterMovement()->MaxWalkSpeed != ChaseSpeed)
	{
		Chase(Pawn);
	}
}

void APS_Enemy::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor->IsA<APS_Character>()) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Character captured!"));
}

float APS_Enemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//UE_LOG(LogTemp, Log, TEXT("Actor : %s took Damage : %f from %s"), *GetName(), FinalDamage, EventInstigator->GetFName());
	return FinalDamage;
}

