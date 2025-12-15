// // Fill out your copyright notice in the Description page of Project Settings.


#include "ComboWindowEnd.h"
#include "PlayerCombatComponent.h"
#include "PlayerCharacter.h"

void UComboWindowEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    UPlayerCombatComponent* Combat = Player->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    // ÄÞº¸ ÀÔ·ÂÃ¢ ´Ý±â
    Combat->bComboWindowOpen = false;

    UE_LOG(LogTemp, Warning, TEXT("[Notify] ComboWindowEnd"));


}