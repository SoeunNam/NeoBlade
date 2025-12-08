// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_PlayAttackMontage.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MeleeEnemy.h"

UBTTask_PlayAttackMontage::UBTTask_PlayAttackMontage()
{
    NodeName = "Play AttackMontage";
}

EBTNodeResult::Type UBTTask_PlayAttackMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    AMeleeEnemy* meleeEnemy = Cast<AMeleeEnemy>(AI->GetPawn());


    if (!meleeEnemy)
    {

        return EBTNodeResult::Failed;
    }

    meleeEnemy->Attack();

    return EBTNodeResult::Succeeded;
}
