// Fill out your copyright notice in the Description page of Project Settings.


#include "HitStunEnd.h"
#include "PlayerCombatComponent.h"



void UHitStunEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
    {
        Combat->EndHitStun();
    }
}
