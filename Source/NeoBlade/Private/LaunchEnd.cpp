// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchEnd.h"
#include "PlayerCombatComponent.h"

void ULaunchEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{

    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    Combat->EndLaunchSkill();
}
