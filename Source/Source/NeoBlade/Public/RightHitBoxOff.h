// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RightHitBoxOff.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API URightHitBoxOff : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 히트박스 이름 지정 (Left, Right 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HitBoxName;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
