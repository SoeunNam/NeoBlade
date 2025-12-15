// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "RoomManager.generated.h"

UCLASS()
class NEOBLADE_API ARoomManager : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ARoomManager();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;


public:
    // ===== [Wave 1] 레벨에 직접 배치한 적들 =====
    UPROPERTY(EditAnywhere, Category = "Wave1")
    TArray<ACharacter*> FirstWaveEnemies;

    // ===== [Wave 2] 스폰용 클래스 =====
    UPROPERTY(EditAnywhere, Category = "Wave2")
    TSubclassOf<ACharacter> RangedEnemyClass;

    UPROPERTY(EditAnywhere, Category = "Wave2")
    TSubclassOf<ACharacter> MeleeEnemyClass;

    // ===== [Wave 2] 스폰 위치 =====
    UPROPERTY(EditAnywhere, Category = "Wave2")
    TArray<AActor*> RangedSpawnPoints;    // 원거리 2개

    UPROPERTY(EditAnywhere, Category = "Wave2")
    TArray<AActor*> MeleeSpawnPoints;     // 근거리 3개

    // ===== 문 앞에서 기다릴 자리 (연출 위치) =====
    UPROPERTY(EditAnywhere, Category = "Entry")
    TArray<AActor*> EntryStandPoints;     // 총 5개 (근3 + 원2 순서 등 네가 정함)

    // ===== 투명벽 (Blocking Volume / BP) =====
    UPROPERTY(EditAnywhere, Category = "Wall")
    AActor* InvisibleWall;

    // 현재 방에 살아있는 적들(현재 웨이브)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<ACharacter*> CurrentEnemies;

    UPROPERTY(EditAnywhere, Category = "AI")
    TSubclassOf<class AAIController> EnemyAIControllerClass;

    UPROPERTY(EditAnywhere, Category = "AI")
    class UBehaviorTree* RangedBehaviorTree;

    // 살아있는 적 수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 AliveEnemyCount = 0;

    // 지금이 1웨이브인지 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    bool bIsFirstWave = true;

public:

    // ===== 웨이브 제어 함수들 =====

    // 1웨이브 시작 (직접 배치한 적 등록)
    void StartFirstWave();

    // 2웨이브 스폰
    void SpawnSecondWave();

    // 적들을 EntryStandPoints까지 걷게 하는 연출 이동
    void MoveEnemiesToEntryPoints();

    // 문이 완전히 열린 시점에 호출 → 전투 AI 활성화 + 벽 해제
    UFUNCTION(BlueprintCallable)
    void ActivateEnemiesAfterDoor();

    // 적이 죽을 때 RoomManager에 알려주는 함수
    UFUNCTION(BlueprintCallable)
    void OnEnemyDead();

    // 투명벽 제거
    void ClearWall();
};
