#include "TargetingComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"            // ★ FOverlapResult 정의 포함
#include "Components/PrimitiveComponent.h" // ★ Hit.GetActor() 사용
#include "CollisionQueryParams.h"          // ★ Params
#include "CollisionShape.h"                // ★ FCollisionShape
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerCharacter.h"               // bCameraLock 접근용
#include "Engine/OverlapResult.h"
#include "Boss1.h"
#include "ADEnemy.h"
#include "MeleeEnemy.h"

UTargetingComponent::UTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTargetingComponent::BeginPlay()
{
    Super::BeginPlay();
}


void UTargetingComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ============================================================
    // ★ 매 프레임 타겟 유지/갱신 + 카메라 락 온/오프 판정
    // ============================================================
    MaintainTarget();

    // 마커는 타겟 유지 중일 때만 위치 갱신
    if (CurrentTarget && TargetMarkerMesh)
        UpdateMarkerPosition();

    // 디버그 표시
    if (GEngine)
    {
        FString Msg = CurrentTarget ?
            FString::Printf(TEXT("TARGET: %s"), *CurrentTarget->GetName()) :
            TEXT("TARGET: NONE");

        GEngine->AddOnScreenDebugMessage(10, 0.f, FColor::Yellow, Msg);
    }




    // ==========================
    // ★ 디버깅: 현재 타겟 표시
    // ==========================
    if (GEngine)
    {
        FString Msg;

        if (CurrentTarget)
        {
            Msg = FString::Printf(TEXT("TARGET: %s | DIST = %.0f"),
                *CurrentTarget->GetName(),
                FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation()));
        }
        else
        {
            Msg = TEXT("TARGET: NONE");
        }

        GEngine->AddOnScreenDebugMessage(
            100,            // Key (업데이트용)
            0.0f,           // Duration 0 = 매 프레임 갱신
            FColor::Yellow,
            Msg
        );
    }


}

//
//// ============================================================
//// ★ 주변에서 가장 적합한 타겟(적) 찾기
////    - 여기선 단순히 "가장 가까운 Pawn" 기준
////    - 나중에 EnemyBase, Boss 우선 등으로 확장 가능
//// ============================================================
//AActor* UTargetingComponent::FindBestTarget()
//{
//    AActor* Owner = GetOwner();
//    if (!Owner) return nullptr;
//
//    FVector Center = Owner->GetActorLocation();
//    TArray<FOverlapResult> Overlaps;
//
//    FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);
//
//    bool bHit = GetWorld()->OverlapMultiByChannel(
//        Overlaps,
//        Center,
//        FQuat::Identity,
//        ECC_Pawn,
//        Sphere
//    );
//
//    if (!bHit) return nullptr;
//
//    float BestDist = FLT_MAX;
//    AActor* BestTarget = nullptr;
//
//    for (auto& Hit : Overlaps)
//    {
//        AActor* Candidate = Hit.GetActor();
//        if (!Candidate || Candidate == Owner) continue;
//
//        // =============================
//        // ★ Enemy / Boss만 타겟 허용
//        // =============================
//        if (!Candidate->IsA<AADEnemy>() &&
//            !Candidate->IsA<ABoss1>() &&
//            !Candidate->IsA<AMeleeEnemy>())
//        {
//            continue;
//        }
//
//        float Dist = FVector::Dist(Center, Candidate->GetActorLocation());
//
//        if (Dist < BestDist)
//        {
//            BestDist = Dist;
//            BestTarget = Candidate;
//        }
//    }
//
//    return BestTarget;
//}
//
//
//// ============================================================
//// ★ 타겟 유지 + 카메라 락 온/오프 처리 핵심
////    - 타겟이 생기면 → PlayerCharacter.bCameraLock = true
////    - 타겟이 사라지거나 멀어지면 → PlayerCharacter.bCameraLock = false
//// ============================================================
//void UTargetingComponent::MaintainTarget()
//{
//    AActor* Owner = GetOwner();
//    APlayerCharacter* PC = Cast<APlayerCharacter>(Owner);
//
//    // 1) 이미 타겟이 있는 경우 → 거리/유효성 체크
//    if (CurrentTarget)
//    {
//        // 타겟이 삭제되었거나 비활성화되면 타겟 해제
//        if (!CurrentTarget->IsValidLowLevelFast() || CurrentTarget->IsPendingKillPending())
//        {
//            CurrentTarget = nullptr;
//            if (PC)
//                PC->bCameraLock = false; // 카메라 락 해제
//            return;
//        }
//        
//     
//
//        float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
//
//        // 락온 유지 반경을 벗어나면 타겟 및 카메라 락 해제
//        if (Dist > LockMaintainRadius)
//        {
//            CurrentTarget = nullptr;
//            if (PC)
//                PC->bCameraLock = false;
//        }
//
//        return; // 아직 타겟 유지 중이면 여기서 끝
//    }
//
//    // 2) 타겟이 없는 경우 → 새 타겟 탐색
//    AActor* NewTarget = FindBestTarget();
//
//    if (NewTarget)
//    {
//        CurrentTarget = NewTarget;
//
//        // ★ 타겟을 새로 잡았으므로 카메라 락 온!
//        if (PC)
//        {
//            PC->bCameraLock = true;
//        }
//
//        // 디버그 로그
//        UE_LOG(LogTemp, Warning,
//            TEXT("[Targeting] New Target Locked: %s"),
//            *NewTarget->GetName());
//    }
//    else
//    {
//        // 주변에 타겟이 없는 상태 → 카메라도 자유 회전 모드
//        if (PC)
//            PC->bCameraLock = false;
//    }
//}

AActor* UTargetingComponent::FindBestTarget()
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    FVector Center = Owner->GetActorLocation();
    TArray<FOverlapResult> Overlaps;

    FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRadius);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Center,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    if (!bHit) return nullptr;

    float BestDist = FLT_MAX;
    AActor* BestTarget = nullptr;

    for (auto& Hit : Overlaps)
    {
        AActor* Candidate = Hit.GetActor();
        if (!Candidate || Candidate == Owner) continue;

        // Enemy / Boss 만 허용
        if (/*!Candidate->IsA<AADEnemy>() &&
            !Candidate->IsA<AMeleeEnemy>() &&*/
            !Candidate->IsA<ABoss1>())
        {
            continue;
        }

        float Dist = FVector::Dist(Center, Candidate->GetActorLocation());

        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestTarget = Candidate;
        }
    }

    return BestTarget;
}
void UTargetingComponent::MaintainTarget()
{
    APlayerCharacter* PC = Cast<APlayerCharacter>(GetOwner());

    // ============================================================
    // 1) 현재 타겟 유지
    // ============================================================
    if (CurrentTarget)
    {
        // --- 1) 타겟이 삭제됨 ---
        if (!CurrentTarget->IsValidLowLevelFast() ||
            CurrentTarget->IsPendingKillPending())
        {
            HideMarker();
            ClearTargetMarker();       // ★ 여기만 OK

            LastTarget = nullptr;
            CurrentTarget = nullptr;

            if (PC) PC->bCameraLock = false;
            return;
        }

        // --- 2) 타겟이 너무 멀어짐 ---
        float Dist = FVector::Dist(
            GetOwner()->GetActorLocation(),
            CurrentTarget->GetActorLocation()
        );

        if (Dist > LockMaintainRadius)
        {
            HideMarker();
            ClearTargetMarker();       // ★ 여기만 OK

            LastTarget = nullptr;
            CurrentTarget = nullptr;

            if (PC) PC->bCameraLock = false;
            return;
        }

        // --- 3) 타겟 유지 상태 (유효 + 거리 OK)
        return;
    }

    // ============================================================
    // 2) 현재 타겟이 없을 때만 새 타겟 탐색
    // ============================================================
    AActor* NewTarget = FindBestTarget();

    // 🙅 절대 ClearTargetMarker() 넣지 마라.
    //    (넣으면 흐름 꼬이고 이전 마커가 남아있을 수 있다)

    if (NewTarget)
    {
        CurrentTarget = NewTarget;

        if (PC)
            PC->bCameraLock = true;

        // --- 마커 생성 ---
        CreateTargetMarker();
        ShowMarker();

        UE_LOG(LogTemp, Warning, TEXT("[Targeting] New Target: %s"),
            *NewTarget->GetName());

        return;
    }

    // --- 타겟 없어도 그냥 PC->bCameraLock 꺼줌 ---
    if (PC)
        PC->bCameraLock = false;
}


void UTargetingComponent::CreateTargetMarker()
{
    if (!CurrentTarget) return;
    if (TargetMarkerMesh) return;   // 이미 생성됨

    TargetMarkerMesh = NewObject<UStaticMeshComponent>(
        CurrentTarget,
        TEXT("TargetMarkerMesh"));

    TargetMarkerMesh->RegisterComponent();
    TargetMarkerMesh->AttachToComponent(
        CurrentTarget->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform
    );

    // 삼각형 메쉬 (임시로 Cone 사용)
    UStaticMesh* ConeMesh = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Engine/BasicShapes/Cone.Cone")
    );

    if (ConeMesh)
        TargetMarkerMesh->SetStaticMesh(ConeMesh);


    // 뾰족 부분 아래로 향하게 회전
    TargetMarkerMesh->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));

    //// 빨간색 머티리얼
    //UMaterialInstanceDynamic* DynMat =
    //    TargetMarkerMesh->CreateAndSetMaterialInstanceDynamic(0);

    //if (DynMat)
    //{
    //    DynMat->SetVectorParameterValue(FName("BaseColor"), FLinearColor::Red);
    //}


    UMaterialInterface* MarkerMat = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Script/Engine.Material'/Game/Player/MT/MT_TargetingMark.MT_TargetingMark'") // <- Content / Player / MT / TagetinMarkClor 경로에 있음
    );

    if (!MarkerMat)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Marker] Failed to load material!"));
    }
    else
    {
        TargetMarkerMesh->SetMaterial(0, MarkerMat);
    }


    TargetMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TargetMarkerMesh->SetCastShadow(false);
    TargetMarkerMesh->SetVisibility(false);

    // 기본 스케일
    TargetMarkerMesh->SetRelativeScale3D(FVector(0.3f));
}

void UTargetingComponent::UpdateMarkerPosition()
{
    if (!CurrentTarget || !TargetMarkerMesh) return;

    // 타겟 캡슐 높이를 이용하면 자동으로 "머리 위" 설정됨
    float Height = 150.f;

    if (UCapsuleComponent* Capsule =
        CurrentTarget->FindComponentByClass<UCapsuleComponent>())
    {
        Height = Capsule->GetScaledCapsuleHalfHeight() * 2.f + 30.f;
    }

    TargetMarkerMesh->SetRelativeLocation(FVector(0, 0, Height));
}

void UTargetingComponent::ShowMarker()
{
    if (TargetMarkerMesh)
        TargetMarkerMesh->SetVisibility(true);
}

void UTargetingComponent::HideMarker()
{
    if (TargetMarkerMesh)
        TargetMarkerMesh->SetVisibility(false);
}

void UTargetingComponent::ClearTargetMarker()
{
    if (TargetMarkerMesh)
    {
        TargetMarkerMesh->DestroyComponent();
        TargetMarkerMesh = nullptr;
    }
}