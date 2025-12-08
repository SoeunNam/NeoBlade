// PlayerDamageComponent.cpp

#include "PlayerDamageComponent.h"
#include "PlayerHealthComponent.h"
#include "PlayerHitReactionComponent.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Actor.h"

UPlayerDamageComponent::UPlayerDamageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerDamageComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    HealthComp = Owner->FindComponentByClass<UPlayerHealthComponent>();
    HitReactionComp = Owner->FindComponentByClass<UPlayerHitReactionComponent>();
    CombatComp = Owner->FindComponentByClass<UPlayerCombatComponent>();

    UE_LOG(LogTemp, Warning, TEXT("[PlayerDamage] BeginPlay | HP=%s, HitReaction=%s, Combat=%s"),
        HealthComp ? TEXT("OK") : TEXT("NULL"),
        HitReactionComp ? TEXT("OK") : TEXT("NULL"),
        CombatComp ? TEXT("OK") : TEXT("NULL"));
}

void UPlayerDamageComponent::DealDamage(float Damage, AActor* Causer)
{
    if (!HealthComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerDamage] HealthComp NULL"));
        return;
    }

    // 1) 체력 감소
    HealthComp->ApplyDamage(Damage);

    // 2) 전투 상태 → HitStun 으로 변경
    if (CombatComp)
    {
        CombatComp->ChangeCombatState(ECombatState::HitStun);
        UE_LOG(LogTemp, Warning, TEXT("[PlayerDamage] CombatState -> HitStun"));
    }

    // 3) 피격 리액션 (애니메이션 트리거)
    if (HitReactionComp)
    {
        HitReactionComp->PlayHitReaction();
    }
}
