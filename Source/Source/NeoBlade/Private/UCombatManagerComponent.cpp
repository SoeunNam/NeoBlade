// Fill out your copyright notice in the Description page of Project Settings.


#include "UCombatManagerComponent.h"

#include "UCombatSlotComponent.h"

// Sets default values for this component's properties
UUCombatManagerComponent::UUCombatManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UUCombatManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUCombatManagerComponent::RequestAttack(UUCombatSlotComponent* Requester)
{
	if (!Requester) return;

	// 이미 공격자면 중복 방지
	if (CurrentAttackers.Contains(Requester))
		return;

	// 이미 대기자면 중복 방지
	if (WaitingEnemies.Contains(Requester))
		return;

	if (CurrentAttackers.Num() < MaxAttackers)
	{
		CurrentAttackers.Add(Requester);
		Requester->SetAttackPermission(true);
	}
	else
	{
		WaitingEnemies.Add(Requester);
		Requester->SetAttackPermission(false);  // ? 명시적으로 막아줌
	}
}

void UUCombatManagerComponent::OnEnemyDeath(UUCombatSlotComponent* DeadEnemy)
{
	// ? 공격 중이던 적 제거
	CurrentAttackers.Remove(DeadEnemy);

	// ? 대기 목록에서도 제거 (안전장치)
	WaitingEnemies.Remove(DeadEnemy);

	// ? 빈 슬롯이 생겼고 + 대기자가 있으면 → 즉시 1명 승급
	if (CurrentAttackers.Num() < MaxAttackers && WaitingEnemies.Num() > 0)
	{
		UUCombatSlotComponent* NextEnemy = WaitingEnemies[0];
		WaitingEnemies.RemoveAt(0);

		CurrentAttackers.Add(NextEnemy);
		NextEnemy->SetAttackPermission(true);   // ? 핵심: 여기서만 true
	}
	UE_LOG(LogTemp, Error, TEXT("? OnEnemyDeath "));
}

