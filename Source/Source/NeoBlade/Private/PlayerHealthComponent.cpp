// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthComponent.h"

// Sets default values for this component's properties
UPlayerHealthComponent::UPlayerHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	MaxHP = 200.f;

	CurrentHP = MaxHP;
}


// Called when the game starts
void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UPlayerHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UPlayerHealthComponent::ApplyDamage(float Damage)
{

	Super::ApplyDamage(Damage);
	OnHPChanged.Broadcast(CurrentHP);
}

void UPlayerHealthComponent::HandleDeath()
{
	if (bisActorDie) return; // 중복 호출 방지

	bisActorDie = true;

	OnPlayerDeath.Broadcast();
	// 플레이어는 Destroy하면 안 됨

	UE_LOG(LogTemp, Warning, TEXT("[PlayerHealth] HandleDeath() CALLED!!!!"));


}