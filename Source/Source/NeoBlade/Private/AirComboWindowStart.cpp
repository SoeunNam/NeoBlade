// Fill out your copyright notice in the Description page of Project Settings.


#include "AirComboWindowStart.h"
#include "PlayerCombatComponent.h"

void UAirComboWindowStart::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = Mesh->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            Combat->OpenAirComboWindow();
        }
    }
}
