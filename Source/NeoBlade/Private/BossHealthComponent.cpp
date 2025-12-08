// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHealthComponent.h"
#include "Boss1.h"
#include "BossHitReactionComponent.h"


// Sets default values for this component's properties
UBossHealthComponent::UBossHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBossHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// 시작 체력 = 최대 체력
	MaxHP = 2700.f;
	CurrentHP = MaxHP;
}


// Called every frame
void UBossHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBossHealthComponent::ApplyDamage(float Damage)
{
	if (Damage <= 0.f) return;

	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("Boss Health: %f"), CurrentHP);

	// 죽었을 경우
	if (CurrentHP <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Dead"));

		HandleDeath();

		return;
	}

	// 피격 리액션 재생
	if (UBossHitReactionComponent* HitReactComp = GetOwner()->FindComponentByClass<UBossHitReactionComponent>())
	{
		HitReactComp->PlayHitReact();
	}
}

void UBossHealthComponent::HandleDeath()
{
	// 보스 주인 가져오기
	if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
	{
		// 보스 상태를 Hit으로 변경
		Boss->SetBossState(EBossState::Dead);

		// Optional: Hit 애니 재생이 ABP에서 State Machine으로 연결되어 있다면, 몽타주 따로 안해도 됨
	}
}

