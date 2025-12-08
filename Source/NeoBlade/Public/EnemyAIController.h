// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class NEOBLADE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
	UBehaviorTree* BehaviorTreeAsset;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// 블랙보드 키 이름 캐싱 (하드코딩 방지)
	const FName Key_TargetActor = FName("TargetActor");
};
