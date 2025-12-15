// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnim.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UEnemyAnim : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	float Speed;
	UPROPERTY(BlueprintReadOnly)
	float Direction;
	UPROPERTY(BlueprintReadOnly)
	bool Circle;

	UPROPERTY(BlueprintReadOnly)
	bool bHasTarget;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead;
	UPROPERTY(BlueprintReadOnly)
	bool bIsAttackingState;
	UFUNCTION()
	void AnimNotify_SlashStart();  // ← Notify 이름과 정확히 맞춰야 한다
	UFUNCTION()
	void AnimNotify_SlashStart2();  // ← Notify 이름과 정확히 맞춰야 한다


	UFUNCTION()
	void AnimNotify_End_Slash();
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
