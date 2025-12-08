#include "HitReactionComponent.h"

UHitReactionComponent::UHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHitReactionComponent::PlayHitReaction()
{
    // 부모는 아무것도 안 함 → 자식이 override
}

void UHitReactionComponent::PlayHitReaction(EAttackType AttackType)
{
    UE_LOG(LogTemp, Warning, TEXT("Base HitReaction (No Animation) Type=%d"), (int32)AttackType);

    // 부모는 아무것도 안 함 → 자식이 override
}
