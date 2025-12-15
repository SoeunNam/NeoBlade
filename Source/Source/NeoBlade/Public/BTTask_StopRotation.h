// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StopRotation.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UBTTask_StopRotation : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_StopRotation();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
