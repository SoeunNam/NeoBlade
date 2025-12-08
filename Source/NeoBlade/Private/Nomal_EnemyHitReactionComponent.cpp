// Fill out your copyright notice in the Description page of Project Settings.
#include "Nomal_EnemyHitReactionComponent.h"
#include "MeleeEnemy.h"                        // Enemy 캐릭터 제어용
#include "GameFramework/Character.h"           // ACharacter 사용
#include "GameFramework/CharacterMovementComponent.h"  // 이동/중력/낙하 상태 체크
#include "Engine/Engine.h"                     // GEngine->AddOnScreenDebugMessage
#include "AttackType.h"                        // EAttackType
#include"ADEnemy.h"




// Sets default values for this component's properties
UNomal_EnemyHitReactionComponent::UNomal_EnemyHitReactionComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UNomal_EnemyHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...

}


void UNomal_EnemyHitReactionComponent::PlayHitReaction()
{
    if (AMeleeEnemy* MeleeEnemy = Cast<AMeleeEnemy>(GetOwner()))
    {
        MeleeEnemy->PlayHitMontage();
    }
    if (AADEnemy* ADEnemy = Cast<AADEnemy>(GetOwner()))
    {
        ADEnemy->PlayHitMontage();
    }
}

// Called every frame
void UNomal_EnemyHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...

    if (!bIsInAirSlam) return;

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    // 바닥에 닿았을 때 Landing 처리
    if (OwnerChar->GetCharacterMovement()->IsMovingOnGround())
    {
        bIsInAirSlam = false;

        UE_LOG(LogTemp, Warning, TEXT("[EnemyAirFinish] Landed! Ground Impact Triggered"));

        // 이펙트와 넉다운 처리
       /* PlayGroundImpactEffect();
        EnterKnockDownState();*/
    }
}


//// 아직 애니메이션이 준비 되지 않았으니 실제 애니메이션 몽타주가 만들어질 때 까지는 주석 처리
//void UNomal_EnemyHitReactionComponent::PlayHitReaction(EAttackType AttackType)
//{
//    UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Reaction Type = %d"), (int32)AttackType);
//
//    switch (AttackType)
//    {
//    case EAttackType::Light:
//        PlayMontage(LightHitMontage);
//        break;
//
//    case EAttackType::Heavy:
//        PlayMontage(HeavyHitMontage);
//        break;
//
//    case EAttackType::Launch:
//        LaunchIntoAir(nullptr);
//        break;
//
//    case EAttackType::AirLight:
//    case EAttackType::AirHeavy:
//        PlayMontage(AirHitMontage);
//        break;
//
//    default:
//        PlayMontage(LightHitMontage);
//        break;
//    }
//}

void UNomal_EnemyHitReactionComponent::PlayHitReaction(EAttackType AttackType)
{
    UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] HitReaction Called | AttackType = %d"), (int32)AttackType);

    if (GEngine)
    {
        FString DebugMsg = FString::Printf(TEXT("Enemy Hit! Type = %s"), *UEnum::GetValueAsString(AttackType));
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, DebugMsg);
    }

    switch (AttackType)
    {
    case EAttackType::Light:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Ground Light Hit"));
        PlayHitReaction();
        break;

    case EAttackType::Heavy:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Ground Heavy Hit"));
        break;

    case EAttackType::Launch:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Launch Hit → Airborne"));
        LaunchIntoAir(nullptr);
        break;

    case EAttackType::AirLight:
    case EAttackType::AirHeavy:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Air Combo Hit → Sustain Airborne"));
        SustainAirborneHit();
        break;

        // ★★★ 추가된 AirFinish 처리 ★★★
    case EAttackType::AirFinish:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Air FINISH HIT → SLAM DOWN"));
        AirFinishHit();
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("[EnemyHitReaction] Default Hit"));
        PlayHitReaction();
        break;
    }
}


//
// // 마찬가지로 애니메이션 준비 될 때 까지는 주석처리
//void UNomal_EnemyHitReactionComponent::LaunchIntoAir(AActor* Causer)
//{
//    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
//    if (!OwnerChar) return;
//
//    float LaunchVelocityZ = 650.f;
//
//    OwnerChar->LaunchCharacter(FVector(0, 0, LaunchVelocityZ), true, true);
//
//    // 공중 피격 리액션 애니메이션 재생
//    PlayMontage(AirHitMontage);
//}

// 마찬가지로 설계 로직이 잘 기능 하고 있는 지 디버깅용 코드
void UNomal_EnemyHitReactionComponent::LaunchIntoAir(AActor* Causer)
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnemyHitReaction] LaunchIntoAir FAILED: Owner is NULL"));
        return;
    }

    // 공중으로 떠오르게 z 위치값 설정 ( 플레이어와 동일하게 850으로 z축 값 설정. )
    float LaunchVelocityZ = 850.f;

    UE_LOG(LogTemp, Warning,
        TEXT("[EnemyHitReaction] LaunchIntoAir() | Owner=%s | Z Velocity=%.1f"),
        *OwnerChar->GetName(), LaunchVelocityZ);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan,
            FString::Printf(TEXT("Enemy Airborne! Z=%0.1f"), LaunchVelocityZ));
    }

    OwnerChar->LaunchCharacter(FVector(0, 0, LaunchVelocityZ), true, true);
}

void UNomal_EnemyHitReactionComponent::SustainAirborneHit()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    FVector CurrentVel = OwnerChar->GetVelocity();
    float AddZ = 300.f;          // 공중 유지 힘
    float MaxHeight = 500.f;     // 적이 너무 높이 안 올라가게 제한

    float OwnerZ = OwnerChar->GetActorLocation().Z;
    float GroundZ = OwnerChar->GetActorLocation().Z - 200.f; // 대충 바닥 기준(필요시 Trace)

    // 너무 높으면 더 안 올림
    if (OwnerZ - GroundZ > MaxHeight)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAir] Height limit reached"));
        return;
    }

    FVector LaunchVel = FVector(CurrentVel.X, CurrentVel.Y, AddZ);

    UE_LOG(LogTemp, Warning,
        TEXT("[EnemyAir] SustainAirborneHit() | Velocity Z=%.1f → %.1f"),
        CurrentVel.Z, LaunchVel.Z);

    OwnerChar->LaunchCharacter(LaunchVel, true, true);
}

void UNomal_EnemyHitReactionComponent::AirFinishHit()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    UE_LOG(LogTemp, Warning, TEXT("[EnemyAirFinish] Air Finish HIT → Slam Down!"));

    float SlamForce = -1600.f;  // 아래 방향 강한 힘

    // 현재 속도를 무시하고 강제로 아래로 밀어내기
    FVector NewVel = FVector(0.f, 0.f, SlamForce);

    OwnerChar->LaunchCharacter(NewVel, true, true);

    // 상태 전환
    if (UMovementComponent* Move = OwnerChar->GetMovementComponent())
    {
        Move->Velocity = NewVel;
    }

    // AI에서 KnockDown 상태로 전환 등 (선택)
    // EnemyState = EEnemyState::KnockDown;

    bIsInAirSlam = true; // 바닥 충돌 감지용
}
