// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackHitEnd.h"
#include "PlayerCombatComponent.h"

void UAttackHitEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            Combat->EndAttackHit();
        }
    }
}
