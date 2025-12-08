// Fill out your copyright notice in the Description page of Project Settings.


#include "Nomal_EnemyDamageComponent.h"
#include "Nomal_EnemyHealthComponent.h"
#include "Nomal_EnemyHitReactionComponent.h"
#include "GameFramework/Actor.h"
#include "MeleeEnemy.h"
#include "ADEnemy.h"

// Sets default values for this component's properties
UNomal_EnemyDamageComponent::UNomal_EnemyDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UNomal_EnemyDamageComponent::BeginPlay()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	Super::BeginPlay();
	HealthComp = Owner->FindComponentByClass<UNomal_EnemyHealthComponent>();
	HitReactionComp = Owner->FindComponentByClass<UNomal_EnemyHitReactionComponent>();
	// ...

}


// Called every frame
void UNomal_EnemyDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UNomal_EnemyDamageComponent::DealDamage(float Damage, AActor* Causer)
{
	AMeleeEnemy* meleeEnemy = Cast<AMeleeEnemy>(GetOwner());
	if (!HealthComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerDamage] HealthComp NULL"));
		return;
	}

	// 1) 체력 감소
	HealthComp->ApplyDamage(Damage);

	if (meleeEnemy)
	{
		meleeEnemy->bIsHit = true;
	}
	if (AADEnemy* ADEnemy = Cast<AADEnemy>(GetOwner()))
	{
		ADEnemy->bIsHit = true;
	}

	// 3) 피격 리액션 (애니메이션 트리거)
	if (HitReactionComp)
	{
		HitReactionComp->PlayHitReaction();
	}
}

void UNomal_EnemyDamageComponent::AttackDealDamage(float Damage, AActor* Causer, EAttackType AttackType)
{
	if (!HealthComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyDamage] HealthComp NULL"));
		return;
	}

	// 1) 체력 감소
	HealthComp->ApplyDamage(Damage);

	// 2) Enemy FSM 에게 피격 상태 전달
	if (AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(GetOwner()))
	{
		Enemy->bIsHit = true;
	}

	// 3) 피격 리액션 실행
	if (HitReactionComp)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EnemyDamage] Call HitReaction | AttackType = %d"),
			(int32)AttackType);

		HitReactionComp->PlayHitReaction(AttackType);
	}

	// 4) 디버깅 로그
	UE_LOG(LogTemp, Warning,
		TEXT("[EnemyDamage] Damage=%.1f | From=%s | AttackType=%d"),
		Damage,
		*GetNameSafe(Causer),
		(int32)AttackType
	);
}


