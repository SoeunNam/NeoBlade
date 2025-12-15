// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_OrbitAroundPlayer.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include"MeleeEnemy.h"

UBTTask_OrbitAroundPlayer::UBTTask_OrbitAroundPlayer()
{
	bNotifyTick = true;   // ✅ TickTask 사용
	NodeName = "Orbit Around Player";
}

EBTNodeResult::Type UBTTask_OrbitAroundPlayer::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	auto AI = OwnerComp.GetAIOwner();
	auto Enemy = Cast<AMeleeEnemy>(AI->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->bIsReady = true;  // 원형 회전 모드 켜기

	return EBTNodeResult::Succeeded;
}

