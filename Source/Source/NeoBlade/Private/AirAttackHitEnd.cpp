// Fill out your copyright notice in the Description page of Project Settings.


#include "AirAttackHitEnd.h"
#include "PlayerCombatComponent.h"

void UAirAttackHitEnd::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = Mesh->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            // 플레이어 공격 히트 판정 종료 신호 전달
            Combat->EndAirAttackHit();
        }
    }
}
