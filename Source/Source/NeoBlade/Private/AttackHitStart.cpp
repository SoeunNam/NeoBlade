// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackHitStart.h"
#include "PlayerCombatComponent.h"

void UAttackHitStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            Combat->StartAttackHit(HitRange, HitRadius, Damage);
        }
    }
}
