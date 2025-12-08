// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetDestination.generated.h"

/**
 *
 */
UCLASS()
class NEOBLADE_API UBTTask_GetDestination : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_GetDestination();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
public:
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 300.f;
	void Debug(FString Msg, FColor Color = FColor::Yellow, float Time = 2.f);
};
