// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackEnd.h"
#include "PlayerCombatComponent.h"
#include "PlayerCharacter.h"
#include "Animation/AnimInstance.h"

void UAttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    UPlayerCombatComponent* Combat = Player->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    // 공격 종료 처리 (콤보 값 리셋 + 상태 초기화)
    Combat->EndAttack();

    // 안전하게 몽타주 정리
    if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
    {
        Anim->Montage_Stop(0.1f);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Notify] AttackEnd"));
}
