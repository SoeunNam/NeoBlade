// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AirAttackEnd.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UAirAttackEnd : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 플레이어 공중 공격 콤보 종료 알림
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
