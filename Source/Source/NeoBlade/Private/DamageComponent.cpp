// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageComponent.h"
#include "PlayerHealthComponent.h"
#include "HitReactionComponent.h"
#include "PlayerCombatComponent.h"


// Sets default values for this component's properties
UDamageComponent::UDamageComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...


}


// Called when the game starts
void UDamageComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...

}


// Called every frame
void UDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}


void UDamageComponent::DealDamage(float Damage, AActor* Causer)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[DamageComponent] Owner NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[DamageComponent] %s takes %.1f damage from %s"),
        *Owner->GetName(),
        Damage,
        Causer ? *Causer->GetName() : TEXT("Unknown")
    );

    // 체력 컴포넌트 찾기
    if (UHealthComponent* HP = Owner->FindComponentByClass<UHealthComponent>())
    {
        HP->ApplyDamage(Damage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DamageComponent] %s has NO HealthComponent!"), *Owner->GetName());
    }

    // 피격 반응 컴포넌트
    if (UHitReactionComponent* HR = Owner->FindComponentByClass<UHitReactionComponent>())
    {
        HR->PlayHitReaction();
    }
}

void UDamageComponent::AttackDealDamage(float Damage, AActor* Causer, EAttackType AttackType)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[DamageComponent] Owner NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[DamageComponent] %s takes %.1f damage from %s (AttackType=%d)"),
        *Owner->GetName(),
        Damage,
        Causer ? *Causer->GetName() : TEXT("Unknown"),
        (int32)AttackType
    );

    // 체력 처리
    if (UHealthComponent* HP = Owner->FindComponentByClass<UHealthComponent>())
    {
        HP->ApplyDamage(Damage);
    }

    // 피격 반응 처리
    if (UHitReactionComponent* HR = Owner->FindComponentByClass<UHitReactionComponent>())
    {
        HR->PlayHitReaction(AttackType);   // AttackType 기반 리액션 전달
    }
}

EHitReactionType UDamageComponent::ConvertAttackToReaction(EAttackType Type)
{
    switch (Type)
    {
    case EAttackType::Light:
        return EHitReactionType::GroundLight;

    case EAttackType::Heavy:
        return EHitReactionType::GroundHeavy;

    case EAttackType::Launch:
        return EHitReactionType::AirborneLaunch;

    case EAttackType::AirLight:
    case EAttackType::AirHeavy:
        return EHitReactionType::AirHit;

        // ★ 패링 타입이 들어오면 ParryStun 반환
    case EAttackType::Parry:
        return EHitReactionType::ParryStun;


    default:
        return EHitReactionType::GroundLight;
    }
}
