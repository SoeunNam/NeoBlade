// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_OrbitAroundPlayer.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UBTTask_OrbitAroundPlayer : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_OrbitAroundPlayer();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
public:
	UPROPERTY(EditAnywhere, Category = "Circle")
	float CircleSpeed = 120.f;   // 도는 속도

	UPROPERTY(EditAnywhere, Category = "Circle")
	float KeepDistanceOffset = 50.f; // AttackRange 바깥 유지값
};
