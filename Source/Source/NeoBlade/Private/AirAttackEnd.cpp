// Fill out your copyright notice in the Description page of Project Settings.


#include "AirAttackEnd.h"
#include "PlayerCombatComponent.h"

void UAirAttackEnd::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = Mesh->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            //플레이어 공중 공격 콤보 종료 알림
            Combat->AnimNotify_AirAttackEnd();
        }
    }
}
