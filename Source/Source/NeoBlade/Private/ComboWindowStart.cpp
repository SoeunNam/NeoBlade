// Fill out your copyright notice in the Description page of Project Settings.


#include "ComboWindowStart.h"
#include "PlayerCombatComponent.h"
#include "PlayerCharacter.h"

void UComboWindowStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());
    if (!Player) return;

    UPlayerCombatComponent* Combat = Player->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    Combat->bComboWindowOpen = true;
    UE_LOG(LogTemp, Warning, TEXT("[Notify] ComboWindowStart"));

    // ? 이미 공격 입력을 저장해둔 경우 → 바로 다음 콤보 실행
    if (Combat->bComboQueued)
    {
        Combat->ContinueCombo();
    }
}

