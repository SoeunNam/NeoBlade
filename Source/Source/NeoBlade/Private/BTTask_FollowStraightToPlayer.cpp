// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FollowStraightToPlayer.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
UBTTask_FollowStraightToPlayer::UBTTask_FollowStraightToPlayer()
{
    bNotifyTick = true;   // Tick 허용
}

EBTNodeResult::Type UBTTask_FollowStraightToPlayer::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;
}

void UBTTask_FollowStraightToPlayer::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return;

    ACharacter* Enemy = Cast<ACharacter>(AI->GetPawn());
    if (!Enemy) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(Enemy, 0);
    if (!Player) return;

    FVector MyLoc = Enemy->GetActorLocation();
    FVector TargetLoc = Player->GetActorLocation();

    float Dist = FVector::Dist(MyLoc, TargetLoc);

    // 일정 거리 이내면 성공 처리
    if (Dist < StopDistance)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 방향 계산 (일자로 직진)
    FVector Dir = (TargetLoc - MyLoc).GetSafeNormal2D();

    // 이동 방향으로 회전
    FRotator TargetRot = Dir.Rotation();
    Enemy->SetActorRotation(FMath::RInterpTo(
        Enemy->GetActorRotation(), TargetRot, DeltaSeconds, 6.f));

    // NavMesh 경로가 아니라 직접 이동
    Enemy->AddMovementInput(Dir, 1.0f);
}

EBTNodeResult::Type UBTTask_FollowStraightToPlayer::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::Aborted;
}
