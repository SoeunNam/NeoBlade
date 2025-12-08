// Fill out your copyright notice in the Description page of Project Settings.


#include "BossDamageComponent.h"
#include "BossHealthComponent.h"
#include "BossHitReactionComponent.h"
#include "Boss1.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UBossDamageComponent::UBossDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBossDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	   // 액터에 붙은 HealthComponent 가져오기
	HealthComp = GetOwner()->FindComponentByClass<UBossHealthComponent>();

	if (!HealthComp)
	{
		UE_LOG(LogTemp, Error, TEXT("BossHealthComponent 없음!"));
	}

	/*
	UE_LOG(LogTemp, Warning, TEXT("[BossDamage] BeginPlay | HP=%s, HitReaction=%s, Combat=%s"),
		HealthComp ? TEXT("OK") : TEXT("NULL"),
		HitReactionComp ? TEXT("OK") : TEXT("NULL"),
		CombatComp ? TEXT("OK") : TEXT("NULL"));
		*/
}



// Called every frame
void UBossDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBossDamageComponent::ApplyBossDamage(float Damage)
{
	// 실제 동작 구현
	UE_LOG(LogTemp, Warning, TEXT("ApplyDamage called: %f"), Damage);

	// 예: BossHealthComponent 참조해서 체력 감소
	if (UBossHealthComponent* FoundHealthComp = GetOwner()->FindComponentByClass<UBossHealthComponent>())
	{
		HealthComp = FoundHealthComp;
		HealthComp->ApplyDamage(Damage);
		
	}

}

