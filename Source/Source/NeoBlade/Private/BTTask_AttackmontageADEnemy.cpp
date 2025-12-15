// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackmontageADEnemy.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ADEnemy.h"

UBTTask_AttackmontageADEnemy::UBTTask_AttackmontageADEnemy()
{
    NodeName = "AttackMontage ADEnemy";
}

EBTNodeResult::Type UBTTask_AttackmontageADEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    AADEnemy* ADEnemy = Cast<AADEnemy>(AI->GetPawn());


    if (!ADEnemy)
    {

        return EBTNodeResult::Failed;
    }

    ADEnemy->Attack();

    return EBTNodeResult::Succeeded;
}
