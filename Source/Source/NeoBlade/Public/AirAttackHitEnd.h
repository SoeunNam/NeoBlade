// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AirAttackHitEnd.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API UAirAttackHitEnd : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 플레이어 공격 히트 판정 종료 신호 전달
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};