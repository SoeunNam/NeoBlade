// Fill out your copyright notice in the Description page of Project Settings.


#include "StopMovement.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

UStopMovement::UStopMovement()
{
    NodeName = "Stop Movement";
}

EBTNodeResult::Type UStopMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;
    // 캐릭터라면 MovementComponent 접근
    ACharacter* Character = Cast<ACharacter>(AI->GetPawn());
    if (Character)
    {
        Character->GetCharacterMovement()->StopMovementImmediately();
        Character->GetCharacterMovement()->MaxWalkSpeed = 0.f;
    }

    return EBTNodeResult::Succeeded;
}