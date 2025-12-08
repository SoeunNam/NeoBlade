// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FollowStraightToPlayer.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API UBTTask_FollowStraightToPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_FollowStraightToPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 추적을 멈출 거리 */
    UPROPERTY(EditAnywhere, Category = "Settings")
    float StopDistance = 120.f;
	
};
