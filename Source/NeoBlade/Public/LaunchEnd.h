// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "LaunchEnd.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API ULaunchEnd : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// 에어본 스킬 종료 노티파이
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
