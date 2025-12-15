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

void UHitReactionComponent::EnterParryStun()
{
    UE_LOG(LogTemp, Warning, TEXT("[HitReaction] Enter Parry Stun"));

    // TODO: 여기에 적 스턴 상태로 전환 로직
    // 예: 
    // CurrentHitState = EHitState::ParryStunned;

    // 자식에서 제정의 가능한 부모의 함수
}
