// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackmontageADEnemy.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UBTTask_AttackmontageADEnemy : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_AttackmontageADEnemy();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
