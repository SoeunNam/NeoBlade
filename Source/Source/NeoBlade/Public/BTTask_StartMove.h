// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartMove.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UBTTask_StartMove : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_StartMove();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ExposeOnSpawn = "true"))
	float Speed = 300.f;

};
