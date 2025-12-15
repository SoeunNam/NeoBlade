// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ADEnemyAnim.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UADEnemyAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	float Speed;
	UPROPERTY(BlueprintReadWrite)
	float A_Pitch;
	UPROPERTY(BlueprintReadWrite)
	float A_Yaw;
	FRotator Rot;

	UFUNCTION()
	void AnimNotify_End_Attack();
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
