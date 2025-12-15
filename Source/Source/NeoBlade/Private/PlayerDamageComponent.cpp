// PlayerDamageComponent.cpp

#include "PlayerDamageComponent.h"
#include "PlayerHealthComponent.h"
#include "PlayerHitReactionComponent.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Actor.h"
#include "GuardShieldComponent.h"

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
//
//void UPlayerDamageComponent::DealDamage(float Damage, AActor* Causer)
//{
//    if (!HealthComp)
//    {
//        UE_LOG(LogTemp, Error, TEXT("[PlayerDamage] HealthComp NULL"));
//        return;
//    }
//
//    // 1) 체력 감소
//    HealthComp->ApplyDamage(Damage);
//
//    // 2) 전투 상태 → HitStun 으로 변경
//    if (CombatComp)
//    {
//        CombatComp->ChangeCombatState(ECombatState::HitStun);
//        UE_LOG(LogTemp, Warning, TEXT("[PlayerDamage] CombatState -> HitStun"));
//    }
//
//    // 3) 피격 리액션 (애니메이션 트리거)
//    if (HitReactionComp)
//    {
//        HitReactionComp->PlayHitReaction();
//    }
//}

void UPlayerDamageComponent::DealDamage(float Damage, AActor* Causer)
{
    if (!CombatComp || !HealthComp || !HitReactionComp)
        return;

    AActor* OwnerActor = GetOwner();

    // ============================================================
    // ★ 0) 이미 죽은 상태면 절대 어떤 반응도 실행되지 않게 함
    // ============================================================
    if (HealthComp->bisActorDie)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Damage] Already Dead → Ignore all damage & hit reactions."));
        return;
    }

    // ============================================================
    // ★ 1) 회피 무적
    // ============================================================
    if (HealthComp->bInvincible)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DODGE] Invincible → Damage ignored."));
        return;
    }

    // ============================================================
    // ★ 2) 패링 성공 체크
    // ============================================================
    if (CombatComp->TryParry(Causer))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Damage] PARRY SUCCESS → No damage, no reaction."));
        return;
    }

    // ============================================================
    // ★ 3) Guard 상태 처리
    // ============================================================
    if (CombatComp->IsGuarding())
    {
        UGuardShieldComponent* Shield = OwnerActor->FindComponentByClass<UGuardShieldComponent>();

        if (!Shield)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Guard] NO Shield → normal damage"));
            HealthComp->ApplyDamage(Damage);

            // ★ ApplyDamage() 후 죽었을 수도 있으므로 다시 체크
            if (HealthComp->bisActorDie) return;

            CombatComp->ChangeCombatState(ECombatState::HitStun);
            HitReactionComp->PlayHitReaction();
            return;
        }

        bool bBlocked = Shield->OnGuardHit(Causer);

        if (bBlocked)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Guard] Shield Block SUCCESS"));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Guard] Shield Block FAILED → Guard Reaction only"));
        HitReactionComp->PlayGuardHitReaction(Causer);
        return;
    }

    // ============================================================
    // ★ 4) 일반 피격
    // ============================================================
    HealthComp->ApplyDamage(Damage);

    // ★ ApplyDamage 후 죽었으면 HitReaction 불가!
    if (HealthComp->bisActorDie)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Damage] Died from this hit → No HitReaction"));
        return;
    }

    // 생존 상태면 HitStun
    CombatComp->ChangeCombatState(ECombatState::HitStun);
    HitReactionComp->PlayHitReaction();
}
