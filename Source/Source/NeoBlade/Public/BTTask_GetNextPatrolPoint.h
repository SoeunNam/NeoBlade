// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetNextPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API UBTTask_GetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetNextPatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector DestinationKey;

	// ¼øÂû ¹Ý°æ
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 500.f;

};
