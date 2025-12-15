// Fill out your copyright notice in the Description page of Project Settings.


#include "AirComboWindowEnd.h"
#include "PlayerCombatComponent.h"

void UAirComboWindowEnd::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = Mesh->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            Combat->CloseAirComboWindow();
        }
    }
}
