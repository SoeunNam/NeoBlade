// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss1.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BossHealthComponent.h"
#include "BossDamageComponent.h"
#include "BossHitReactionComponent.h"
#include "DrawDebugHelpers.h"
#include "AttackType.h"
#include "GameFramework/Character.h"
#include "PlayerDamageComponent.h"

// Sets default values
ABoss1::ABoss1()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // 왼팔 히트박스 생성
    LeftArmHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArmHitBox"));
    LeftArmHitBox->SetupAttachment(GetMesh(), TEXT("index_01_lSocket"));
    LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftArmHitBox->SetCollisionObjectType(ECC_Pawn);
    LeftArmHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    LeftArmHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 오른팔 히트박스
    RightArmHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArmHitBox"));
    RightArmHitBox->SetupAttachment(GetMesh(), TEXT("index_01_rSocket"));
    RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightArmHitBox->SetCollisionObjectType(ECC_Pawn);
    RightArmHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    RightArmHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 전방 충격파(스파크 웨이브)
    ShockWaveHitBox = CreateDefaultSubobject<USphereComponent>(TEXT("ShockWaveHitBox"));
    ShockWaveHitBox->SetupAttachment(RootComponent);
    ShockWaveHitBox->SetSphereRadius(120.f);
    ShockWaveHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShockWaveHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    ShockWaveHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 회전 공격용 히트박스
    SpinAttackHitBox = CreateDefaultSubobject<USphereComponent>(TEXT("SpinAttackHitBox"));
    SpinAttackHitBox->SetupAttachment(RootComponent);
    SpinAttackHitBox->SetSphereRadius(200.f);
    SpinAttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpinAttackHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    SpinAttackHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 점프 착지 충격파용 히트박스
    LandingHitBox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LandingHitBox"));
    LandingHitBox->SetupAttachment(RootComponent);
    LandingHitBox->SetCapsuleHalfHeight(60.f);
    LandingHitBox->SetCapsuleRadius(120.f);
    LandingHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LandingHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    LandingHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    auto* Move = GetCharacterMovement();
    Move->GravityScale = 2.2f;               // 더 무겁게
    Move->JumpZVelocity = 480.f;             // 점프 힘
    Move->AirControl = 0.4f;                 // 공중 이동 반응
    Move->FallingLateralFriction = 1.5f;     // 떨어질 때 둔함 감소

    HealthComp = CreateDefaultSubobject<UBossHealthComponent>(TEXT("HealthComp"));
    DamageComp = CreateDefaultSubobject<UBossDamageComponent>(TEXT("DamageComp"));
    HitReactionComp = CreateDefaultSubobject<UBossHitReactionComponent>(TEXT("HitReactionComp"));

}

// Called when the game starts or when spawned
void ABoss1::BeginPlay()
{
    Super::BeginPlay();

    CurrentHP = MaxHP;

    // 플레이어 포인터 가져오기
    Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (LeftArmHitBox && GetMesh())
    {
        LeftArmHitBox->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TEXT("index_01_lSocket")
        );
    }

    if (RightArmHitBox && GetMesh())
    {
        RightArmHitBox->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TEXT("index_01_rSocket")
        );
    }

    if (ShockWaveHitBox && GetMesh())
    {
        ShockWaveHitBox->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TEXT("pelvis")
        );
    }
    LeftArmHitBox->OnComponentBeginOverlap.AddDynamic(this, &ABoss1::OnHitBoxOverlap);
    RightArmHitBox->OnComponentBeginOverlap.AddDynamic(this, &ABoss1::OnHitBoxOverlap);
    ShockWaveHitBox->OnComponentBeginOverlap.AddDynamic(this, &ABoss1::OnHitBoxOverlap);
    SpinAttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &ABoss1::OnHitBoxOverlap);
    LandingHitBox->OnComponentBeginOverlap.AddDynamic(this, &ABoss1::OnHitBoxOverlap);
}

// Called every frame
void ABoss1::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // === DEBUG HITBOX VISUALIZATION ===

    if (LeftArmHitBox && LeftArmHitBox->IsCollisionEnabled())
        DrawDebugBox(GetWorld(), LeftArmHitBox->GetComponentLocation(), LeftArmHitBox->GetScaledBoxExtent(),
            LeftArmHitBox->GetComponentQuat(), FColor::Red, false, -1.f, 0, 2.f);

    if (RightArmHitBox && RightArmHitBox->IsCollisionEnabled())
        DrawDebugBox(GetWorld(), RightArmHitBox->GetComponentLocation(), RightArmHitBox->GetScaledBoxExtent(),
            RightArmHitBox->GetComponentQuat(), FColor::Red, false, -1.f, 0, 2.f);

    if (ShockWaveHitBox && ShockWaveHitBox->IsCollisionEnabled())
        DrawDebugSphere(GetWorld(), ShockWaveHitBox->GetComponentLocation(),
            ShockWaveHitBox->GetScaledSphereRadius(), 16, FColor::Blue, false, -1.f, 0, 2.f);

    if (SpinAttackHitBox && SpinAttackHitBox->IsCollisionEnabled())
        DrawDebugSphere(GetWorld(), SpinAttackHitBox->GetComponentLocation(),
            SpinAttackHitBox->GetScaledSphereRadius(), 16, FColor::Yellow, false, -1.f, 0, 2.f);

    if (LandingHitBox && LandingHitBox->IsCollisionEnabled())
        DrawDebugCapsule(GetWorld(), LandingHitBox->GetComponentLocation(),
            LandingHitBox->GetScaledCapsuleHalfHeight(), LandingHitBox->GetScaledCapsuleRadius(),
            LandingHitBox->GetComponentQuat(), FColor::Green, false, -1.f, 0, 2.f);

    // ------------------
    // 디버그: 상태 출력 (매 프레임 — 개발 중에만)
    //UE_LOG(LogTemp, Warning, TEXT("[TICK] State:%d bIsAttacking:%d bPatternOnCooldown:%d bCanUseSkill:%d bCanNormalAttack:%d"),
       // (int)BossState, bIsAttacking, bPatternOnCooldown, bCanUseSkill, bCanNormalAttack);

    if (!Player || BossState == EBossState::Dead) return;

    if (bIsIdlePaused) return;

    if (BossState != EBossState::Dead)
    {
    // 플레이어 바라보기
    FVector Direction = Player->GetActorLocation() - GetActorLocation();
    Direction.Z = 0;
    FRotator TargetRotation = Direction.Rotation();
    FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.f);
    SetActorRotation(NewRotation);
    }

    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    // Hit 모션 재생 중이면 공격 로직 스킵
    if (BossState == EBossState::Hit)
    {
        return;
    }
    if (BossState == EBossState::PhaseChange)
    {
        return;
	}
    if (BossState == EBossState::Stunned)
    {
        return;
    }
    if (bIsAttacking || bPatternOnCooldown)
    {
        // 공격 중이거나 쿨타임이면 상태 유지
        return;
    }

    if (BossState != EBossState::Hit && BossState != EBossState::PhaseChange)
    {
        // 3. 행동 결정 (이동 vs 공격)
        if (Dist > AttackRange)
        {
            // 이동 상태
            SetBossState(EBossState::Jog);
            MoveTowardsPlayer(DeltaTime);
        }
        else
        {
            // 공격 상태 (공격 범위 내)
            // bCanUseSkill, bCanNormalAttack 등의 내부 쿨타임이 없다면
            // 여기서 바로 공격을 시도합니다.
            // TryAttack()은 스킬/일반 공격 쿨타임을 내부적으로 체크하므로 사용 가능
            SetBossState(EBossState::Attacking);
            TryAttack();
        }
    }
    if (bIsChargeMoving)
    {
        FVector Forward = GetActorForwardVector();
        float MoveSpeed = 600.f; // 원하는 만큼 튀어나가게 조절

        AddMovementInput(Forward, MoveSpeed * DeltaTime);
    }
}

// Called to bind functionality to input
void ABoss1::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}



/// <summary>
/// 외부에서 데미지 받을 때 호출되는 함수
/// </summary>
/// <param name="Amount"></param>
void ABoss1::ApplyDamage(float Amount)
{
    //UE_LOG(LogTemp, Warning, TEXT("ApplyDamage %f called. PrevHP:%f"), Amount, CurrentHP);
    CurrentHP -= Amount;

    if (CurrentHP <= 0)
    {
        BossState = EBossState::Dead;
        // TODO: 사망 애니메이션 및 이벤트 실행

        // 사망 애니/이펙트 호출 (예시)
        // PlayDeathMontage();
        // Disable collisions, disable movement 등
        GetCharacterMovement()->DisableMovement();
        // 적절히 히트박스 비활성화
        DeactivateHitBox("Left");
        DeactivateHitBox("Right");
        DeactivateHitBox("Shock");
        DeactivateHitBox("Spin");
        DeactivateHitBox("Landing");

        // 모든 공격/패턴 타이머 정리(선택)
        GetWorldTimerManager().ClearTimer(SkillTimer);
        GetWorldTimerManager().ClearTimer(NormalAttackTimer);
        GetWorldTimerManager().ClearTimer(SpinTimer);


        return;
    }



    // 페이즈 체크
    CheckPhaseChange();
}


/// <summary>
/// HP 50% 이하에서 페이즈 2로 전환
/// </summary>
void ABoss1::CheckPhaseChange()
{
    if (CurrentPhase == 1 && CurrentHP <= 1350.0f)
    {
        CurrentPhase = 2;
        BossState = EBossState::PhaseChange;

        // TODO: 페이즈 전환 연출 (카메라, 이펙트)
        // 일정 시간 후 다시 패턴 가능 상태로 변경
        GetWorldTimerManager().SetTimer(
            PhaseChangeTimer,
            this,
            &ABoss1::EndPattern,
            3.0f,
            false
        );

        TryAttack();
    }

}
/// <summary>
///  거리 기반 + 랜덤 기반으로 다음 패턴 선택
/// </summary>
void ABoss1::SelectNextPattern()
{
    BossState = EBossState::Attacking;

    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());


    int32 Rand = FMath::RandRange(0, 100);

    UE_LOG(LogTemp, Warning, TEXT("SelectNextPattern Phase:%d Dist:%.1f AttackRange:%.1f Rand:%d"),
        CurrentPhase, Dist, AttackRange, Rand);

    //  PHASE 1
    if (CurrentPhase == 1)
    {
        switch (Phase1PatternIndex)
        {
        case 0:
            StartPattern(EBossPattern::DashCharge);  // 주먹 날리기
            break;

        case 1:
            StartPattern(EBossPattern::JumpShock);   // 점프 스매시
            break;

        case 2:
            StartPattern(EBossPattern::SparkShock);  // 스파크 파동
            break;
        }

        // 다음 패턴으로 이동 (0→1→2→0)
        Phase1PatternIndex = (Phase1PatternIndex + 1) % 3;
        return;
    }

    // PHASE 2
    if (CurrentPhase == 2)
    {
        switch (Phase2PatternIndex)
        {
        case 0:
            StartPattern(EBossPattern::SpinAttack);   // 회전 팔 공격
            break;

        case 1:
            StartPattern(EBossPattern::DashCharge);    // 앞으로 주먹    
            break;

        case 2:
            StartPattern(EBossPattern::SparkShock2);  // 강화 스파크
            break;
        }

        Phase2PatternIndex = (Phase2PatternIndex + 1) % 3;
        return;
    }
}

/// <summary>
/// 패턴 실행 시작
/// </summary>
/// <param name="Type"></param>
void ABoss1::StartPattern(EBossPattern Type)
{
    CurrentPattern = Type;
    bIsAttacking = true;

    // 패턴 시작하면 쿨타임 + IdlePause 막기 활성화
   // bPatternOnCooldown = true;
    //bIsIdlePaused = true;

    switch (Type)
    {
    case EBossPattern::DashCharge:
        Pattern_Charge();
        break;

    case EBossPattern::SparkShock:
        Pattern_SparkWave();
        break;

    case EBossPattern::JumpShock:
        Pattern_JumpSmash();
        break;

    case EBossPattern::SpinAttack:
        Pattern_SpinAttack();
        break;

    case EBossPattern::SparkShock2:
        Pattern_SparkWave2();
        break;

    default:
        break;
    }
}

/// <summary>
/// 패턴 종료 처리 (공격 완료 후 호출됨)
/// </summary>
void ABoss1::EndPattern()
{
    //UE_LOG(LogTemp, Warning, TEXT("EndPattern called. CurrentPattern:%d"), (int)CurrentPattern);
    bIsAttacking = false;
    CurrentPattern = EBossPattern::None;
   // UE_LOG(LogTemp, Warning, TEXT("EndPattern finished. bIsAttacking:%d"), bIsAttacking);

    //한 번만 2초 Idle
    StartIdlePause(5.0f);

	LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/*
    아래는 각 패턴 함수 (지금은 애니메이션만 재생하는 뼈대)
    실제 히트 판정은 Montage Notify에서 처리하면 됨
*/

/// <summary>
/// 돌격 패턴
/// </summary>
void ABoss1::Pattern_Charge()
{
    UE_LOG(LogTemp, Warning, TEXT("ChargeDash"));
    // TODO: 돌격 애니메이션 몽타주 재생
    // Notify: DashStart / DashHit / DashEnd
  // Optional: SpinAttack 몽타주 재생
    if (ChargeMontage)
    {
        PlayAnimMontage(ChargeDashMontage);
    }

	FTimerHandle ChargeMoveStartTimer;
    // 1.4초 뒤에 이동 시작
    GetWorldTimerManager().SetTimer(
        ChargeMoveStartTimer,
        this,
        &ABoss1::StartChargeMove,
        1.4f,
        false
    );

	FTimerHandle ChargeMoveStopTimer;
    // 1.9초 뒤에 이동 끝내기 (총 0.5초 동안)
    GetWorldTimerManager().SetTimer(
        ChargeMoveStopTimer,
        this,
        &ABoss1::StopChargeMove,
        1.9f,
        false
    );
}

/// <summary>
/// 양팔 내려찍기 2회
/// </summary>
void ABoss1::Pattern_DoubleSmash()
{
    UE_LOG(LogTemp, Warning, TEXT("DoubleSmash"));
    // TODO: DoubleSmash 몽타주 재생
    // Notify1: 첫 번째 찍기 판정
    // Notify2: 두 번째 찍기 판정
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->Montage_Play(DoubleSmashMontage);
    }

   // EndPattern();

}

/// <summary>
/// 전방 스파크 충격파
/// </summary>
void ABoss1::Pattern_SparkWave()
{
    UE_LOG(LogTemp, Warning, TEXT("SparkWave"));
    // TODO: SparkShock 몽타주 재생
    // Notify에서 전방 원뿔 전기 충격파 Trace

   // 몽타주 재생
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->Montage_Play(SparkWaveMontage);
    }
  //  EndPattern();
}

/// <summary>
/// 점프 후 착지 충격파
/// </summary>
void ABoss1::Pattern_JumpSmash()
{
    UE_LOG(LogTemp, Warning, TEXT("JumpSmash"));
    if (!JumpSmashMontage || !Player) return;

    
    FVector PlayerLoc = Player->GetActorLocation();
    FVector Forward = Player->GetActorForwardVector();

    // 플레이어 앞 150cm 지점
    FVector LandingPoint = PlayerLoc - Forward * 200.f;
    LandingPoint.Z = GetActorLocation().Z;

    // 점프 방향 벡터
    FVector Launch = LandingPoint - GetActorLocation();
    Launch.Z = 1200.f;          // 점프 고도
    Launch *= 1.0f;            // 속도 증가

    // 몽타주 재생
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->Montage_Play(JumpSmashMontage);
    }

	ActivateHitBox("Landing");

    LaunchCharacter(Launch, true, true);

    FTimerHandle TimerHandle_DeactivateHitbox;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_DeactivateHitbox,
        FTimerDelegate::CreateLambda([this]()
            {
                DeactivateHitBox("Landing");
            }),
        2.0f,   // 지연 시간 (초)
        false   // 반복 X
    );

    // 착지 후 히트박스 처리
    // AnimNotify에서 LandingHitOn / LandingHitOff 호출

   // EndPattern();
}

/// <summary>
/// 회전 팔 공격
/// </summary>
void ABoss1::Pattern_SpinAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("SpinAttack"));
    // 회전 초기화
    RotationDuration = 0.5f;
    RotationTime = 0.f;

    // 회전 시작 기준 Yaw
    StartYaw = GetActorRotation().Yaw;

    // 회전 간격 (프레임처럼 0.01초마다)
    float TickInterval = 0.01f;

    // Optional: SpinAttack 몽타주 재생
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->Montage_Play(SpinAttackMontage);
    }

    // Timer를 이용해 회전 처리
    // 1.5초 후에 실제 회전 시작
    FTimerHandle DelayTimer;
    GetWorldTimerManager().SetTimer(
        DelayTimer,
        FTimerDelegate::CreateLambda([this]()
            {
                // 회전 초기화
                RotationDuration = 1.5f;
                RotationTime = 0.f;

                // 회전 시작 기준 Yaw
                StartYaw = GetActorRotation().Yaw;

                float TickInterval = 0.01f;

                // 회전 Timer
                GetWorldTimerManager().SetTimer(
                    SpinTimer,
                    FTimerDelegate::CreateLambda([this, TickInterval]()
                        {
                            RotationTime += TickInterval;
                            float Alpha = FMath::Clamp(RotationTime / RotationDuration, 0.f, 1.f);

                            FRotator NewRotation = GetActorRotation();
                            NewRotation.Yaw = StartYaw + Alpha * 8640.f; // 6바퀴 회전
                            SetActorRotation(NewRotation);

                            if (Alpha >= 1.f)
                            {
                                GetWorldTimerManager().ClearTimer(SpinTimer);
                            }
                        }),
                    TickInterval,
                    true
                );

            }),
        1.0f, // 1.5초 딜레이
        false
    );
   // EndPattern();
}

void ABoss1::Pattern_SparkWave2()
{
    UE_LOG(LogTemp, Warning, TEXT("SparkWave2"));
    // TODO: SparkShock 몽타주 재생
    // Notify에서 전방 원뿔 전기 충격파 Trace

   // 몽타주 재생
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->Montage_Play(SparkWaveMontage2);
    }
    //  EndPattern();
}

/// <summary>
/// 히트박스 ON 함수
/// 애니메이션 노티파이에서 호출하게 됨
/// </summary>
/// <param name="HitBoxName"></param>
void ABoss1::ActivateHitBox(FName HitBoxName)
{
    if (HitBoxName == "Left")
    {
        LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        if (CurrentPattern == EBossPattern::DashCharge) CurrentActiveHitBoxSkill = EBossSkillHitBox::DashCharge;
        else if (CurrentPattern == EBossPattern::SpinAttack) CurrentActiveHitBoxSkill = EBossSkillHitBox::SpinAttack;
        else CurrentActiveHitBoxSkill = EBossSkillHitBox::None;
    }
    else if (HitBoxName == "Right")
    {
        RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        if (CurrentPattern == EBossPattern::SpinAttack) CurrentActiveHitBoxSkill = EBossSkillHitBox::SpinAttack;
        else if (CurrentPattern == EBossPattern::JumpShock) CurrentActiveHitBoxSkill = EBossSkillHitBox::JumpShock;
        else if (CurrentPattern == EBossPattern::SparkShock) CurrentActiveHitBoxSkill = EBossSkillHitBox::SparkWave;
        else CurrentActiveHitBoxSkill = EBossSkillHitBox::None;
    }
    else if (HitBoxName == "Shock")
    {
        ShockWaveHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    else if (HitBoxName == "Spin")
    {
        SpinAttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    else if (HitBoxName == "Landing")
    {
        LandingHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        if (CurrentPattern == EBossPattern::JumpShock) CurrentActiveHitBoxSkill = EBossSkillHitBox::JumpShock;
        else CurrentActiveHitBoxSkill = EBossSkillHitBox::None;
    }
}
/// <summary>
/// 히트박스 OFF 함수
/// 애니메이션 노티파이에서 호출하게 됨
/// </summary>
/// <param name="HitBoxName"></param>
void ABoss1::DeactivateHitBox(FName HitBoxName)
{
    if (HitBoxName == "Left")
        LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    else if (HitBoxName == "Right")
        RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    else if (HitBoxName == "Shock")
        ShockWaveHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    else if (HitBoxName == "Spin")
        SpinAttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    else if (HitBoxName == "Landing")
        LandingHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CurrentActiveHitBoxSkill = EBossSkillHitBox::None;
}


/// <summary>
/// 보스 상태 전환 함수
/// </summary>
/// <param name="NewState"></param>
void ABoss1::SetBossState(EBossState NewState)
{
    if (BossState == EBossState::Stunned &&
        NewState != EBossState::Dead)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[BossState] Blocked %d -> %d (STUNNED)"),
            (int)BossState, (int)NewState);
        return;
    }


    // 상태가 같으면 무시
    if (BossState == NewState)
        return;

    UE_LOG(LogTemp, Warning, TEXT("[SetBossState] %d -> %d"), (int)BossState, (int)NewState);

    BossState = NewState;

    // 전환 시 특정 작업 실행
    switch (BossState)
    {
    case EBossState::Idle:
        UE_LOG(LogTemp, Warning, TEXT("Idle"));
        break;

    case EBossState::Jog:
        UE_LOG(LogTemp, Warning, TEXT("Jog"));
        break;

    case EBossState::Hit:
        UE_LOG(LogTemp, Warning, TEXT("Hit"));
        break;

    case EBossState::Attacking:
        UE_LOG(LogTemp, Warning, TEXT("Attacking"));
        break;

    case EBossState::PhaseChange:
        UE_LOG(LogTemp, Warning, TEXT("PhaseChange"));
        break;

    case EBossState::Dead:
        UE_LOG(LogTemp, Warning, TEXT("Dead"));
        break;
    }
}

/// <summary>
/// PC 추적 이동
/// </summary>
/// <param name="DeltaTime"></param>
void ABoss1::MoveTowardsPlayer(float DeltaTime)
{
    if (!Player || BossState == EBossState::Dead) return;
    if (!Player || CurrentPattern == EBossPattern::SpinAttack) return;

    BossState = EBossState::Jog;

    // UE_LOG(LogTemp, Warning, TEXT("%d"), BossState);

    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

    if (Dist <= AttackRange) return; // 공격 범위 안이면 멈춤

    FVector Direction = Player->GetActorLocation() - GetActorLocation();
    Direction.Z = 0;
    Direction.Normalize();

    AddMovementInput(Direction, 1.0f);
}

/// <summary>
/// 근접 공격 가능 거리 체크
/// </summary>
void ABoss1::CheckAttack()
{
    if (!Player || bIsAttacking) return;

    float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    if (Dist <= AttackRange)
    {
        TryAttack();
    }
}

/// <summary>
/// 일반 공격 시작
/// </summary>
void ABoss1::StartAttack()
{
    bIsAttacking = true;
    BossState = EBossState::Attacking;

    //UE_LOG(LogTemp, Warning, TEXT("%d"),BossState);


}

/// <summary>
/// 일반공격 종료
/// </summary>
void ABoss1::StopAttack()
{
   // UE_LOG(LogTemp, Warning, TEXT("StopAttack called. PrevState:%d"), (int)BossState);
    bIsAttacking = false;
	StartIdlePause(2.0f); // 1초 대기
    //UE_LOG(LogTemp, Warning, TEXT("StopAttack finished. State:%d bIsAttacking:%d"), (int)BossState, bIsAttacking);
    LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoss1::ResetNormalAttack()
{
    bCanNormalAttack = true;
   // UE_LOG(LogTemp, Warning, TEXT("ResetNormalAttack -> bCanNormalAttack = true"));
}

void ABoss1::ResetSkill()
{
    bCanUseSkill = true;
   // UE_LOG(LogTemp, Warning, TEXT("ResetSkill -> bCanUseSkill = true"));
}

void ABoss1::TryAttack()
{
    // 페이즈 체크
    CheckPhaseChange();

    // PhaseChange 상태면 바로 중단 (절대 공격 패턴 진입 금지)
    if (BossState == EBossState::PhaseChange)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAttack aborted due to PhaseChange"));
        return;
    }

    // 스킬 우선
    if (bCanUseSkill)
    {
        UseSkillAttack();
        return;
    }

    // 스킬은 못쓰면 일반 공격
    if (bCanNormalAttack)
    {
        UseNormalAttack();
        return;
    }

    // 둘 다 쿨타임이면 아무것도 안함
}

/// <summary>
/// 일반공격하기
/// </summary>
void ABoss1::UseNormalAttack()
{
    bCanNormalAttack = false;

    StartAttack();

    // 2초 쿨타임
    GetWorldTimerManager().SetTimer(
        NormalAttackTimer,
        this,
        &ABoss1::ResetNormalAttack,
        NormalAttackCooldown,
        false
    );

    //UE_LOG(LogTemp, Warning, TEXT("일반 공격!"));
}

/// <summary>
/// 스킬공격하기
/// </summary>
void ABoss1::UseSkillAttack()
{
    bCanUseSkill = false;

    SelectNextPattern();

    // 12초 쿨타임
    GetWorldTimerManager().SetTimer(
        SkillTimer,
        this,
        &ABoss1::ResetSkill,
        SkillCooldown,
        false
    );

    //UE_LOG(LogTemp, Warning, TEXT("Skill %d"), CurrentPattern);

}

void ABoss1::HitReactPlay()
{
    // 공격 중이면 끊기게
    bIsAttacking = false;
    bCanUseSkill = false;
    bCanNormalAttack = false;

	LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoss1::StartChargeMove()
{
    bIsChargeMoving = true;
}

void ABoss1::StopChargeMove()
{
    bIsChargeMoving = false;
}

void ABoss1::StopPhaseChange()
{
    // PhaseChange 끝 → Idle로 복귀
    SetBossState(EBossState::Idle);

    // 패턴 재시작 가능
    bIsAttacking = false;
    bPatternOnCooldown = false;

    UE_LOG(LogTemp, Warning, TEXT("[PhaseChange] Phase change ended → Idle"));
}

void ABoss1::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (!PlayerPawn) return;

    float DamageToApply = 0.f;

    UE_LOG(LogTemp, Warning, TEXT("[Overlap] HitBox = %s"), *OverlappedComp->GetName());
    UE_LOG(LogTemp, Warning, TEXT("[Overlap] CurrentSkill = %d"), (int)CurrentActiveHitBoxSkill);

    switch (CurrentActiveHitBoxSkill)
    {
    case EBossSkillHitBox::DashCharge:
        DamageToApply = DashChargeDamage;
        break;

    case EBossSkillHitBox::SparkWave:
        DamageToApply = SparkWaveDamage;
        break;

    case EBossSkillHitBox::SparkWave2:
        DamageToApply = SparkWave2Damage;
        break;

    case EBossSkillHitBox::SpinAttack:
        DamageToApply = SpinAttackDamage;
        break;

    case EBossSkillHitBox::JumpShock:
        DamageToApply = JumpShockDamage;
        break;

    case EBossSkillHitBox::None:
        // 일반 공격 왼손
        if (OverlappedComp == LeftArmHitBox)
            DamageToApply = LeftArmDamageNoramal;

        // 일반 공격 오른손
        else if (OverlappedComp == RightArmHitBox)
            DamageToApply = RightArmDamageNormal;

        break;
    }

    if (DamageToApply <= 0.f) return;

    if (UPlayerDamageComponent* PlayerDamage = PlayerPawn->FindComponentByClass<UPlayerDamageComponent>())
    {
        PlayerDamage->DealDamage(DamageToApply, this);
    }

    UE_LOG(LogTemp, Warning, TEXT("HitBox Hit! Damage = %f"), DamageToApply);

    if (DamageToApply == SpinAttackDamage || DamageToApply == JumpShockDamage)
    {
        return;
    }
    OverlappedComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/// <summary>
/// 공격 후 일정 시간 동안 Idle 상태를 유지하게 하는 함수
/// </summary>
/// <param name="Duration">대기 시간 (초)</param>
void ABoss1::StartIdlePause(float Duration)
{
    if (Duration <= 0.0f)
    {
        StopIdlePause();
        return;
    }

    // 강제 Idle 상태 시작
    bIsIdlePaused = true;
    SetBossState(EBossState::Idle); // 상태를 Idle로 설정

    // 지정된 Duration 후에 StopIdlePause 호출
    GetWorldTimerManager().SetTimer(
        IdlePauseTimer,
        this,
        &ABoss1::StopIdlePause,
        Duration,
        false
    );
    UE_LOG(LogTemp, Warning, TEXT("[IdlePause] Start %f seconds pause."), Duration);
}

/// <summary>
/// IdlePause 상태를 해제하고 일반 Idle/추적 로직으로 복귀
/// </summary>
void ABoss1::StopIdlePause()
{
    // 타이머가 작동 중이면 강제 해제
    GetWorldTimerManager().ClearTimer(IdlePauseTimer);

    // 강제 Idle 상태 해제
    bIsIdlePaused = false;

    UE_LOG(LogTemp, Warning, TEXT("[IdlePause] Pause Ended. Resume normal AI logic."));
    // 상태는 Tick에서 자동으로 Jog 또는 Attack으로 전환될 것입니다.
}
/// <summary>
/// 플레이어 패링 성공 시 보스 스턴
/// </summary>
/// <param name="Duration"></param>
void ABoss1::EnterStunned(float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("[Boss] Enter STUNNED"));

    // === [핵심 수정 부분] IdlePause/StopAttack 타이머를 스턴 시작 시 무조건 해제 ===
    GetWorldTimerManager().ClearTimer(IdlePauseTimer);
    // 기존 공격의 '공격 끝 처리'가 지연되는 것을 방지합니다.
    // =========================================================================

    // 1. 상태 변경
    BossState = EBossState::Stunned;

    // 2. [핵심] 현재 재생 중인 공격 몽타주 즉시 중단
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->StopAllMontages(0.2f); // 0.2초 블렌드 아웃하며 중단

        if (StunnedMontage)
        {
            // 스턴 몽타주 재생. Duration은 타이머가 관리하므로, 
            // 몽타주 재생 속도를 조정하여 Duration과 맞출 필요가 있습니다.
            float PlayRate = StunnedMontage->GetPlayLength() / Duration;

            // 만약 몽타주 재생 시간을 Duration에 맞추려면 아래 코드를 사용 (선택 사항)
            // PlayRate = StunMontage->GetPlayLength() / Duration;

            AnimInst->Montage_Play(StunnedMontage, 1.0f); // PlayRate는 1.0f로 시작하고, 
            // 애니메이션을 스턴 시간(Duration)에 맞게 제작하는 것이 더 편합니다.
        }
    }

    // 3. [핵심] 이동 즉시 정지 (관성 제거)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement(); // 스턴 중 이동 불가 처리
    }

    // 4. [핵심] 켜져있던 공격 히트박스 모두 끄기
    // (이게 없으면 스턴 걸린 상태의 주먹에 닿아도 플레이어가 데미지를 입음)
    DeactivateHitBox("Left");
    DeactivateHitBox("Right");
    DeactivateHitBox("Shock");
    DeactivateHitBox("Spin");
    DeactivateHitBox("Landing");

    // 5. 공격 관련 플래그 초기화
    bIsAttacking = false;
    bIsChargeMoving = false;
    CurrentPattern = EBossPattern::None;

    // 6. 타이머 재설정 (기존 패턴 타이머들이 꼬이지 않게 클리어 해주는 것이 안전)
    GetWorldTimerManager().ClearTimer(SpinTimer);
    //GetWorldTimerManager().ClearTimer(ChargeMoveStartTimer); // 헤더에 핸들 선언 필요

    // 스턴 해제 타이머 설정
    GetWorldTimerManager().ClearTimer(StunTimer);
    GetWorldTimerManager().SetTimer(
        StunTimer,
        this,
        &ABoss1::ExitStunned,
        Duration,
        false
    );

    // [추가] 타이머 설정 성공 여부 및 남은 시간 로그
    if (GetWorldTimerManager().IsTimerActive(StunTimer))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Boss] StunTimer SET SUCCESS. Remaining: %.2f"),
            GetWorldTimerManager().GetTimerRemaining(StunTimer));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Boss] StunTimer SET FAILED!"));
    }
}

void ABoss1::ExitStunned()
{
    // 이미 죽었거나 다른 상태라면 무시
    if (BossState != EBossState::Stunned) return;

    UE_LOG(LogTemp, Warning, TEXT("[Boss] Exit STUNNED"));

    // 1. 이동 다시 활성화
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    // 2. 상태를 Idle로 복귀
    //SetBossState(EBossState::Idle);
	BossState = EBossState::Idle;
   
    // 3. 패턴/공격 플래그 강제 초기화 [수정 및 추가된 부분]
    bIsAttacking = false;
    bPatternOnCooldown = false; // <<< 이 플래그를 false로 설정해줘야 합니다.
    //bCanUseSkill = true;        // <<< 쿨타임도 즉시 리셋 (필요 시)
    //bCanNormalAttack = true;    // <<< 쿨타임도 즉시 리셋 (필요 시)

    // bIsIdlePaused 상태도 초기화 해주는 것이 안전합니다.
    bIsIdlePaused = false;

    // 모든 IdlePause 타이머도 이 시점에서 확실히 정리 (안전 조치)
    GetWorldTimerManager().ClearTimer(IdlePauseTimer);
}
