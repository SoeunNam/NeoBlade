// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttackHitStart.generated.h"

/**
 * 타격 판정 시작 Notify
 * 애니메이션의 특정 프레임에서 SphereTrace를 활성화한다.
 */
UCLASS()
class NEOBLADE_API UAttackHitStart : public UAnimNotify
{
	GENERATED_BODY()
	
public:
    // 타격 판정 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HitRange = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HitRadius = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 10.f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
