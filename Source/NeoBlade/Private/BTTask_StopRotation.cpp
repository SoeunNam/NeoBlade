// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StopRotation.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UBTTask_StopRotation::UBTTask_StopRotation()
{
    NodeName = "Stop Rotation";
}

EBTNodeResult::Type UBTTask_StopRotation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    ACharacter* Character = Cast<ACharacter>(AI->GetPawn());
    if (!Character) return EBTNodeResult::Failed;

    // È¸Àü ¸ØÃß±â
    Character->bUseControllerRotationYaw = false;
    Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;

    return EBTNodeResult::Succeeded;
}
