// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UHealthComponent::ApplyDamage(float Damage)
{
    if (bInvincible)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[HealthComponent] %s is INVINCIBLE -> Damage Ignored"),
            *GetOwner()->GetName());
        return;
    }

    if (Damage <= 0.f) return;

    float OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, MaxHP);

    UE_LOG(LogTemp, Warning,
        TEXT("[HealthComponent] %s HP: %.1f -> %.1f (%.1f Damage)"),
        *GetOwner()->GetName(),
        OldHP,
        CurrentHP,
        Damage);

    if (CurrentHP <= 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[HealthComponent] %s DIED!"),
            *GetOwner()->GetName());

        HandleDeath();
    }
}

void UHealthComponent::HandleDeath()
{
    // 기본 구현 (자식이 override 가능)
    if (AActor* Owner = GetOwner())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[HealthComponent] %s DIED (Default HandleDeath)"),
            *Owner->GetName());

        Owner->Destroy();
    }
}
