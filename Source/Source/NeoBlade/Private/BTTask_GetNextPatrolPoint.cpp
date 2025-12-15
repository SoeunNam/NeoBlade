// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GetNextPatrolPoint.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GetNextPatrolPoint::UBTTask_GetNextPatrolPoint()
{
    NodeName = TEXT("GetNextPatrolPoint");
}

EBTNodeResult::Type UBTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogTemp, Warning, TEXT("GetNextPatrolPoint Execute START"));
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    APawn* Pawn = AI->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    // 1) 네비게이션 시스템 얻기
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    // 2) 현재 위치
    FVector Origin = Pawn->GetActorLocation();

    FNavLocation ResultLocation;



    bool bSuccess = NavSystem->GetRandomPointInNavigableRadius(
        Origin,
        PatrolRadius,      // float
        ResultLocation     // FNavLocation&
    );
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Patrol point NOT found! NavMesh 문제!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Patrol point FOUND: %s"), *ResultLocation.Location.ToString());
    }
    if (!bSuccess)
        return EBTNodeResult::Failed;

    // Blackboard 저장
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    BB->SetValueAsVector(DestinationKey.SelectedKeyName, ResultLocation.Location);

    return EBTNodeResult::Succeeded;
}
