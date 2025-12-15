// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomManager.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnemyAIController.h"
#include"MeleeEnemy.h"
#include"ADEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

ARoomManager::ARoomManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARoomManager::BeginPlay()
{
    Super::BeginPlay();

    // 시작할 때 벽은 닫혀 있게
    if (InvisibleWall)
    {
        InvisibleWall->SetActorEnableCollision(true);
        InvisibleWall->SetActorHiddenInGame(false);
    }

    // 첫 웨이브 시작 (직접 배치한 애들)
    StartFirstWave();
}

// ===================== 1웨이브: 배치형 적 =====================

void ARoomManager::StartFirstWave()
{
    bIsFirstWave = true;
    CurrentEnemies.Empty();
    AliveEnemyCount = 0;

    for (ACharacter* Enemy : FirstWaveEnemies)
    {
        if (!Enemy) continue;

        CurrentEnemies.Add(Enemy);
        AliveEnemyCount++;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Room] First Wave Start: %d enemies"), AliveEnemyCount);
}

// ===================== 2웨이브: 스폰 =====================

void ARoomManager::SpawnSecondWave()
{


    bIsFirstWave = false;
    CurrentEnemies.Empty();

    AliveEnemyCount = 0;

    UWorld* World = GetWorld();
    if (!World) return;


    // ---- 원거리 2명 스폰 ----
    const int32 MaxRanged = FMath::Min(2, RangedSpawnPoints.Num());

    for (int32 i = 0; i < MaxRanged; i++)
    {
        if (!RangedEnemyClass || !RangedSpawnPoints[i]) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FVector SpawnLoc = RangedSpawnPoints[i]->GetActorLocation();
        SpawnLoc.Z += 5.f;   // 80 ❌ → 5 ✅

        ACharacter* ADEnemy = World->SpawnActor<ACharacter>(
            RangedEnemyClass,
            SpawnLoc,
            RangedSpawnPoints[i]->GetActorRotation(),
            Params
        );

        if (ADEnemy)
        {

            // ✅ AI 강제 비활성화 1단계: Controller 강제 생성
            ADEnemy->SpawnDefaultController();

            if (AAIController* AI = Cast<AAIController>(ADEnemy->GetController()))
            {
                // ✅ AI 100% 정지
                AI->StopMovement();

                if (AI->BrainComponent)
                {
                    AI->BrainComponent->StopLogic(TEXT("Wave2Entry"));
                }
            }


            CurrentEnemies.Add(ADEnemy);
            AliveEnemyCount++;
        }
        /*  if (!ADEnemy->GetController())
          {
              AEnemyAIController* NewAI = GetWorld()->SpawnActor<AEnemyAIController>(EnemyAIControllerClass);
              if (NewAI)
              {
                  NewAI->Possess(ADEnemy);
                  UE_LOG(LogTemp, Warning, TEXT("✅ Forced Possess Ranged: %s"), *ADEnemy->GetName());
              }
          }*/




        UE_LOG(LogTemp, Warning, TEXT("✅ Ranged Full Init Restored: %s"), *ADEnemy->GetName());

        // 🔴 🔴 🔴 여기까지 🔴 🔴 🔴
    }

    // ---- 근거리 3명 스폰 ----
    const int32 MaxMelee = FMath::Min(3, MeleeSpawnPoints.Num());

    for (int32 i = 0; i < MaxMelee; i++)
    {
        if (!MeleeEnemyClass || !MeleeSpawnPoints[i]) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* MeleeEnemy = World->SpawnActor<ACharacter>(
            MeleeEnemyClass,
            MeleeSpawnPoints[i]->GetActorLocation() + FVector(0, 0, 80),  // ✅ Z + 80
            MeleeSpawnPoints[i]->GetActorRotation(),
            Params
        );


        if (MeleeEnemy)
        {/*
            MeleeEnemy->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

            if (AAIController* AI = Cast<AAIController>(MeleeEnemy->GetController()))
            {
                AI->StopMovement();
                if (AI->BrainComponent)
                {
                    AI->BrainComponent->StopLogic(TEXT("Wave2Entry"));
                }
            }*/
            // ✅ AI 강제 비활성화 1단계: Controller 강제 생성
            MeleeEnemy->SpawnDefaultController();

            CurrentEnemies.Add(MeleeEnemy);
            AliveEnemyCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Room] Second Wave Spawned: %d enemies"), AliveEnemyCount);

    // 스폰 끝 → 문 앞 대기 위치까지 연출 이동
    MoveEnemiesToEntryPoints();
}

// ===================== 연출 이동 (문 앞까지 걸어나오기) =====================

void ARoomManager::MoveEnemiesToEntryPoints()
{
    const int32 Count = FMath::Min(CurrentEnemies.Num(), EntryStandPoints.Num());

    UE_LOG(LogTemp, Warning, TEXT("✅ MoveEnemiesToEntryPoints: %d"), Count);

    for (int32 i = 0; i < Count; i++)
    {
        ACharacter* Enemy = CurrentEnemies[i];
        AActor* StandPoint = EntryStandPoints[i];

        if (!Enemy || !StandPoint)
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Enemy or StandPoint NULL at %d"), i);
            continue;
        }

        // ✅ 컨트롤러 없으면 강제 생성 (이게 핵심)
        if (!Enemy->GetController())
        {
            Enemy->SpawnDefaultController();
            UE_LOG(LogTemp, Warning, TEXT("✅ Spawned Controller for %s"), *Enemy->GetName());
        }

        AAIController* AI = Cast<AAIController>(Enemy->GetController());
        if (!AI)
        {
            UE_LOG(LogTemp, Error, TEXT("❌ AIController NULL: %s"), *Enemy->GetName());
            continue;
        }

        FVector GoalLoc = StandPoint->GetActorLocation();

        FAIMoveRequest Req;
        Req.SetGoalLocation(GoalLoc);
        Req.SetAcceptanceRadius(25.f);
        Req.SetUsePathfinding(true);

        EPathFollowingRequestResult::Type Result = AI->MoveTo(Req);
        AI->BrainComponent->StartLogic();

        if (Result == EPathFollowingRequestResult::Failed)
        {
            UE_LOG(LogTemp, Error, TEXT("❌ MoveTo Failed → Teleport: %s"), *Enemy->GetName());

            Enemy->SetActorLocation(GoalLoc + FVector(0, 0, 10));
        }
        if (AI && AI->BrainComponent)
        {
            AI->BrainComponent->StartLogic();
        }
        FTimerHandle RestoreCollisionTimer;

        GetWorld()->GetTimerManager().SetTimer(
            RestoreCollisionTimer,
            [Enemy]()
            {
                if (Enemy && Enemy->GetCapsuleComponent())
                {
                    UCapsuleComponent* Capsule = Enemy->GetCapsuleComponent();
                    UCharacterMovementComponent* MoveComp = Enemy->GetCharacterMovement();

                    // ✅ 캡슐 콜리전 100% 복구
                    Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    Capsule->SetCollisionProfileName(TEXT("Pawn"));


                }
            },
            1.0f,     // 이동 연출 끝나는 시간
            false
        );
        UE_LOG(LogTemp, Warning, TEXT("Enemy: %s | HasMovement: %d | Mode: %d"),
            *Enemy->GetName(),
            Enemy->GetCharacterMovement() != nullptr,
            Enemy->GetCharacterMovement() ? (int32)Enemy->GetCharacterMovement()->MovementMode : -1
        );
    }
    ActivateEnemiesAfterDoor();
}

// ===================== 문 열림 이후: 전투 시작 + 벽 해제 =====================

void ARoomManager::ActivateEnemiesAfterDoor()
{
    // 문이 완전히 열린 타이밍(애니 끝)에서 BP나 C++로 이 함수 호출

    for (ACharacter* Enemy : CurrentEnemies)
    {
        if (!Enemy) continue;

        if (AAIController* AI = Cast<AAIController>(Enemy->GetController()))
        {
            if (AI->BrainComponent)
            {
                AI->BrainComponent->RestartLogic(); // 여기서부터 추적 시작
            }
        }
    }

    ClearWall();

    UE_LOG(LogTemp, Warning, TEXT("[Room] Door opened, enemies activated"));
}

// ===================== 적 사망 처리 =====================

void ARoomManager::OnEnemyDead()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.0f,
            FColor::Red,
            FString::Printf(TEXT("Dead"))
        );
    }
    AliveEnemyCount--;

    UE_LOG(LogTemp, Warning, TEXT("[Room] Enemy dead, left: %d"), AliveEnemyCount);

    if (AliveEnemyCount > 0)
        return;



    // 여기까지 오면 현재 웨이브 클리어
    if (bIsFirstWave)
    {


        // 1웨이브 클리어 → 문 연출 + 2웨이브 스폰
        // 문 BP에서 애니 재생 후, 적 타이밍 맞춰 SpawnSecondWave() 호출해도 되고
        // 여기서 바로 스폰 시작해도 됨. 일단 기본형은 바로 스폰:
        SpawnSecondWave();
    }
    else
    {
        // 2웨이브 이후 클리어: 다음 방 열기, 보상 등 여기서 처리
        UE_LOG(LogTemp, Warning, TEXT("[Room] All waves cleared!"));
    }
}

// ===================== 투명벽 해제 =====================

void ARoomManager::ClearWall()
{
    if (InvisibleWall)
    {
        InvisibleWall->SetActorEnableCollision(false);
        InvisibleWall->SetActorHiddenInGame(true);
    }
}