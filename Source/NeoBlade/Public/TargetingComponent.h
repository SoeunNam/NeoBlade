// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

class AActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UTargetingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UTargetingComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:


    // ============================================================
    // ★ 1) 타겟 탐색 반경
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float SearchRadius = 300.f;

    // ============================================================
    // ★ 2) 락온 유지 반경
    //    - 이 거리 밖으로 나가면 타겟 해제
    // ============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
    float LockMaintainRadius = 450.f;

    // ============================================================
    // ★ 3) 현재 타겟(적)
    // ============================================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget = nullptr;

    /** 🔥 새로 추가해야 하는 변수 — 이전 타겟 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    AActor* LastTarget = nullptr;


private:
    // 가장 좋은 타겟 찾기(가장 가까운 적 등)
    AActor* FindBestTarget();

    // 타겟 유지/갱신 + 카메라 락온 신호 처리
    void MaintainTarget();



    // ===== 타겟 마커 =====
    UPROPERTY()
    UStaticMeshComponent* TargetMarkerMesh = nullptr;

    void CreateTargetMarker();     // 처음 타겟 잡을 때 메쉬 생성
    void UpdateMarkerPosition();   // 타겟 머리 위로 이동
    void ShowMarker();
    void HideMarker();

    // 대상 변경 되거나 대상이 없을 때 마크 없애기
    void ClearTargetMarker();


};
