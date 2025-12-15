// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "StopMovement.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UStopMovement : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UStopMovement();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
