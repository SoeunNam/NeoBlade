// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 1. Behavior Tree 에셋이 있는지 확인하고 실행
    // (BTAsset은 에디터에서 할당하거나 생성자에서 로드)
    if (BehaviorTreeAsset)
    {
        // RunBehaviorTree 함수가 내부적으로 UseBlackboard를 호출하여
        // BlackboardComponent를 생성하고 초기화해줍니다.
        RunBehaviorTree(BehaviorTreeAsset);
    }

    // 2. 이제 블랙보드가 생성되었으므로 값을 세팅합니다.
    // (블루프린트에서 했던 Set Value as Object 작업을 여기서 수행)
    if (GetBlackboardComponent())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            GetBlackboardComponent()->SetValueAsObject(Key_TargetActor, PlayerPawn);
        }
    }
}
