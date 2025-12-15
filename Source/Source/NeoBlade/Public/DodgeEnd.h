// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DodgeEnd.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API UDodgeEnd : public UAnimNotify
{
	GENERATED_BODY()

public:

	// 호출될 함수
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	
};
