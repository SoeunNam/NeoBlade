// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AirAttackHitStart.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UAirAttackHitStart : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 플레이어 공격 히트 판정 시작 신호 전달
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
