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
// Sets default values
ASwordTrailActor::ASwordTrailActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword"));
	RootComponent = SwordMesh;

	TrailEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	TrailEffect->SetupAttachment(SwordMesh);
	TrailEffect->SetAutoActivate(false);   // 자동 활성화 X
}

// Called when the game starts or when spawned
void ASwordTrailActor::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AMeleeEnemy>(GetOwner());
}

// Called every frame
void ASwordTrailActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TrailEffect && TrailEffect->IsActive())
	{
		// 소켓 위치 가져오기
		const FVector TipLocation = SwordMesh->GetSocketLocation("Weapon_Tip");
		const FVector BaseLocation = SwordMesh->GetSocketLocation("Weapon_Base");

		// Niagara에 전달
		TrailEffect->SetVariableVec3(TipParam, TipLocation);
		TrailEffect->SetVariableVec3(BaseParam, BaseLocation);
	}
}
void ASwordTrailActor::EnableTrail()
{
	if (TrailEffect)
	{
		TrailEffect->Activate(true);
	}
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

	DrawDebugLine(GetWorld(), BaseLoc, TipLoc, FColor::Green, false, 1.0f, 0, 2.0f);

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

