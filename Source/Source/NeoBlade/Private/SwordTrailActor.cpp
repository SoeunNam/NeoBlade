// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordTrailActor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include"MeleeEnemy.h"
#include"Nomal_EnemyDamageComponent.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"


// Sets default values
ASwordTrailActor::ASwordTrailActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword"));
	RootComponent = SwordMesh;


	TrailComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX"));
	TrailComp->SetupAttachment(RootComponent);
	TrailComp->SetAutoActivate(false);

}

// Called when the game starts or when spawned
void ASwordTrailActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] BeginPlay START"));

	OwnerCharacter = Cast<AMeleeEnemy>(GetOwner());
	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] OwnerCharacter: %s"),
		OwnerCharacter ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] TrailFXComponent: %s"),
		TrailComp ? TEXT("OK") : TEXT("NULL"));

	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] TrailFXSystem: %s"),
		TrailFX ? TEXT("OK") : TEXT("NULL"));
	OwnerCharacter = Cast<AMeleeEnemy>(GetOwner());


	if (TrailComp && TrailFX)
	{
		TrailComp->SetAsset(TrailFX);  // ✅ 여기서 연결
		TrailComp->Activate();             // ✅ 시작은 꺼둠
	}

	UE_LOG(LogTemp, Warning, TEXT("Sword BeginPlay - Trail Ready"));
}

// Called every frame
void ASwordTrailActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!TrailComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] TrailFXComponent NULL in Tick"));
		return;
	}

	if (!TrailComp->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] TrailFXComponent NOT Active"));
		return;
	}

	if (!SwordMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] SwordMesh NULL"));
		return;
	}

	if (TrailEffect && TrailEffect->IsActive())
	{
		// 소켓 위치 가져오기
		const FVector TipLocation = SwordMesh->GetSocketLocation("Weapon_Tip");
		const FVector BaseLocation = SwordMesh->GetSocketLocation("Weapon_Base");

		// Niagara에 전달
		TrailEffect->SetVariableVec3("User_Start", TipLocation);
		TrailEffect->SetVariableVec3("End", BaseLocation);
	}
}
void ASwordTrailActor::EnableTrail()
{
	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] EnableTrail CALLED"));

	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] Component Valid: %s"),
		TrailComp ? TEXT("YES") : TEXT("NO"));

	if (!TrailComp) return;

	TrailComp->Activate();

	UE_LOG(LogTemp, Warning, TEXT("[SwordTrail] IsActive After Activate: %s"),
		TrailComp->IsActive() ? TEXT("TRUE") : TEXT("FALSE"));
}

void ASwordTrailActor::DisableTrail()
{
	if (TrailEffect)
	{
		TrailEffect->Deactivate();
	}
}

void ASwordTrailActor::SwordLineTrace()
{
	AMeleeEnemy* enemy = Cast<AMeleeEnemy>(GetOwner());

	FVector BaseLoc = SwordMesh->GetSocketLocation("Weapon_Base");
	FVector TipLoc = SwordMesh->GetSocketLocation("Weapon_Tip");


	// ✅ 나이아가라 활성화
	if (!TrailComp->IsActive())
	{
		TrailComp->Activate();
	}

	// ✅ 나이아가라 위치 세팅
	TrailComp->SetVariableVec3("User_Start", BaseLoc);
	TrailComp->SetVariableVec3("End", TipLoc);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		BaseLoc,
		TipLoc,
		ECC_Pawn,
		Params
	);
	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
	}
	/*DrawDebugLine(GetWorld(), BaseLoc, TipLoc, FColor::Green, false, 1.0f, 0, 2.0f);*/

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();      // ← 🔥 맞은 액터

		if (HitActor)
		{
			if (HitActor->ActorHasTag("Player"))
			{
				if (UDamageComponent* Dmg = HitActor->FindComponentByClass<UDamageComponent>())
				{
					Dmg->DealDamage(Damage, OwnerCharacter);
				}
			}
		}
	}
}

void ASwordTrailActor::StopTrail()
{
	if (TrailComp)
	{
		TrailComp->Deactivate();
	}
}
