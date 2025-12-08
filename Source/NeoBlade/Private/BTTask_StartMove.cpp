// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StartMove.h"
#include "AIController.h"
#include "MeleeEnemy.h"
#include"ADEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

UBTTask_StartMove::UBTTask_StartMove()
{
    NodeName = "StartMove";
}

EBTNodeResult::Type UBTTask_StartMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;
    // 캐릭터라면 MovementComponent 접근
    if (AMeleeEnemy* Character = Cast<AMeleeEnemy>(AI->GetPawn()))
    {
        Character->SetSpeed(Speed);
        Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
        Character->bUseControllerRotationYaw = true;
        Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;

    }
    if (AADEnemy* Character = Cast<AADEnemy>(AI->GetPawn()))
    {
        Character->SetSpeed(Speed);
        Character->GetCharacterMovement()->MaxWalkSpeed = Speed;
        Character->bUseControllerRotationYaw = true;
        Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;

    }

    return EBTNodeResult::Succeeded;

}

