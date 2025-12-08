// Fill out your copyright notice in the Description page of Project Settings.


#include "DodgeEnd.h"
#include "PlayerCombatComponent.h"
#include "PlayerCharacter.h"


void UDodgeEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    Combat->EndDodge();
}
