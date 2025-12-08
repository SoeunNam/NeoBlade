// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GetDestination.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"MeleeEnemy.h"
#include"ADEnemy.h"

UBTTask_GetDestination::UBTTask_GetDestination()
{
    NodeName = "GetDestination";
}

EBTNodeResult::Type UBTTask_GetDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    APawn* Pawn = AI->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    ACharacter* Character = nullptr;

    if (AMeleeEnemy* Melee = Cast<AMeleeEnemy>(Pawn))
    {
        Character = Melee;
    }
    else if (AADEnemy* AD = Cast<AADEnemy>(Pawn))
    {
        Character = AD;
    }

    if (!Character) return EBTNodeResult::Failed;




    // 패트롤 속도
    Character->GetCharacterMovement()->MaxWalkSpeed = 300.f;

    // 네비게이션
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    FVector Origin = Character->GetActorLocation();

    FNavLocation FoundLoc;
    bool bSuccess = NavSys->GetRandomReachablePointInRadius(
        Origin,
        PatrolRadius,
        FoundLoc
    );

    if (!bSuccess)
        return EBTNodeResult::Failed;

    // 블랙보드 저장
    OwnerComp.GetBlackboardComponent()->SetValueAsVector("Destination", FoundLoc.Location);

    return EBTNodeResult::Succeeded;
}

void UBTTask_GetDestination::Debug(FString Msg, FColor Color, float Time)
{

}
