// Fill out your copyright notice in the Description page of Project Settings.


#include "Nomal_EnemyHealthComponent.h"
#include "MeleeEnemy.h"
#include "ADEnemy.h"

// Sets default values for this component's properties
UNomal_EnemyHealthComponent::UNomal_EnemyHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UNomal_EnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	MaxHP = 50.f;
}


// Called every frame
void UNomal_EnemyHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UNomal_EnemyHealthComponent::ApplyDamage(float Damage)
{
	Super::ApplyDamage(Damage);
	OnNomal_EnemyHPChanged.Broadcast(CurrentHP);
}

void UNomal_EnemyHealthComponent::HandleDeath()
{
	OnNomal_EnemyDeath.Broadcast();
	// 플레이어는 Destroy하면 안 됨

	bisActorDie = true;
	if (AMeleeEnemy* meleeEnemy = Cast<AMeleeEnemy>(GetOwner()))
	{
		meleeEnemy->isEnemyDead = true;
		meleeEnemy->Die();
	}
	if (AADEnemy* ADEnemy = Cast<AADEnemy>(GetOwner()))
	{
		ADEnemy->isEnemyDead = true;
		ADEnemy->Die();
	}
}

