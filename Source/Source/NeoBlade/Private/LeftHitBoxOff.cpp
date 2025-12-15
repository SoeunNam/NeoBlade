// Fill out your copyright notice in the Description page of Project Settings.


#include "LeftHitBoxOff.h"
#include "Boss1.h"

void ULeftHitBoxOff::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    // 애니메이션을 재생한 캐릭터 가져오기
    if (ABoss1* Boss = Cast<ABoss1>(MeshComp->GetOwner()))
    {
        // Boss 클래스의 함수 호출
        Boss->DeactivateHitBox("Left");
    }
}
