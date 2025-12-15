// Fill out your copyright notice in the Description page of Project Settings.


#include "AirAttackHitStart.h"
#include "PlayerCombatComponent.h"

void UAirAttackHitStart::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = Mesh->GetOwner())
    {
        if (UPlayerCombatComponent* Combat = Owner->FindComponentByClass<UPlayerCombatComponent>())
        {
            // 플레이어 공격 히트 판정 시작 신호 전달
            Combat->StartAirAttackHit(150.f, 50.f, 10.f); // 데미지 등ㅡ 원하는 값 넣기
        }
    }
}
