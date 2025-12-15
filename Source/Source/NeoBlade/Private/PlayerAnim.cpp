#include "PlayerAnim.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCombatComponent.h"

void UPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    auto Player = Cast<APlayerCharacter>(TryGetPawnOwner());
    if (!Player) return;

    // 이동 속도
    FVector Velocity = Player->GetVelocity();
    FVector Forward = Player->GetActorForwardVector();
    FVector Right = Player->GetActorRightVector();

    speed = FVector::DotProduct(Forward, Velocity);
    direction = FVector::DotProduct(Right, Velocity);

    // 점프 상태
    if (Player->GetCharacterMovement())
    {
        bIsFalling = Player->GetCharacterMovement()->IsFalling();
    }

    bIsDoubleJumping = Player->bIsDoubleJumping;
    JumpCount = Player->JumpCount;

    // ============================
    // Combat Component 가져오기
    // ============================
    if (!Combat)
    {
        Combat = Player->FindComponentByClass<UPlayerCombatComponent>();
    }

    if (Combat)
    {
        AnimCombatState = Combat->CombatState;
        AnimAttackType = Combat->AttackType;  // ? 추가하면 AnimGraph에서 공격 종류도 사용 가능

    }




}

void UPlayerAnim::PlayAnimMontageSafe(UAnimMontage* Montage, FName SectionName)
{
    if (!Montage) return;

    Montage_Play(Montage, 1.f);

    if (SectionName != NAME_None)
    {
        Montage_JumpToSection(SectionName, Montage);
    }
}

void UPlayerAnim::PlayDodgeMontage()
{
    if (!DodgeMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("[Anim] DodgeMontage is NULL!"));
        return;
    }

    Montage_Play(DodgeMontage, 1.0f);
}

void UPlayerAnim::OnHitReaction()
{
    if (HitReactMontage)
    {
        Montage_Play(HitReactMontage);
    }
}

void UPlayerAnim::PlayGuardHitMontage()
{
    if (GuardHitMontage)
        Montage_Play(GuardHitMontage);
}

void UPlayerAnim::PlayDieMontage()
{
    if (DeathMontage)
        Montage_Play(DeathMontage);
}

