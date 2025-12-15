// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AirComboWindowStart.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UAirComboWindowStart : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 공중 콤보 공격 입력 열림 신호 전달
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
