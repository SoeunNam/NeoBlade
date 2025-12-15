// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerDamageComponent.h"
#include "Boss1.generated.h"

class UBossDamageComponent;
class UBossHitReactionComponent;
class UBossHealthComponent;

/// <summary>
/// 보스 상태, 현재 행동이 무엇인지 나타내는 상태머신
/// </summary>
UENUM(BlueprintType)
enum class EBossState : uint8
{
    Idle,           //패턴없음, 대기, 0
    Jog,			//플레이어 추적 중, 1
    Hit,            //경직, 넉백, 2 
    Attacking,      //공격 중, 3     
    PhaseChange,    //페이즈 전환 연출 중, 4
    Dead,           //사망, 5
    Stunned,        //경직, 패링 당함 6 
};


/// <summary>
/// 보스 패턴 종류, 6개패턴+일반공격
/// </summary>
UENUM(BlueprintType)
enum class EBossPattern : uint8
{
    None,
    DashCharge,     // 직선 전진 돌격, 1
    SparkShock,     // 전방 스파크 충격파, 2
    JumpShock,      // 점프 후 착지 충격파, 3
    SpinAttack,    // 회전 공격, 4
    SparkShock2,	// 강화된 전방 스파크 충격파, 5
};

//현재 스킬 상태를 저장할 변수
UENUM(BlueprintType)
enum class EBossSkillHitBox : uint8
{
    None,
    DashCharge,
    SparkWave,
    JumpShock,
    SpinAttack,
    SparkWave2
};
EBossSkillHitBox CurrentActiveHitBoxSkill = EBossSkillHitBox::None;

UCLASS()
class NEOBLADE_API ABoss1 : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ABoss1();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // ==============================================================
    // 보스 기본 스탯
    // ==============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stats")
    float MaxHP = 2700;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
    float CurrentHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
    int32 CurrentPhase = 1;   // Phase 1 → Phase 2

    // ==============================================================
    // 보스 상태 / 패턴 관리
    // ==============================================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    EBossState BossState = EBossState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    EBossPattern CurrentPattern = EBossPattern::None;

    // 공격 중인지 여부
    bool bIsAttacking = false;

    // 패턴 선택 쿨타임 활성화 여부
    bool bPatternOnCooldown = false;

    // 패턴 종료용 타이머
    FTimerHandle PhaseChangeTimer;

    // 패턴 쿨타임 타이머
    FTimerHandle PatternCooldownTimer;

    //일반 공격 종료 타이머
    FTimerHandle AttackTimer;

    // ==============================================================
    // AI 관련 값
    // ==============================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|AI")
    float DetectRange = 1500.f;   // 패턴 시작하는 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|AI")
    float AttackRange = 200.f;    // 근접 패턴 거리

    // 플레이어 캐시
    APawn* Player;

    // ==============================================================
    // 패턴 선택 및 실행 함수
    // ==============================================================
    void SelectNextPattern();             // 다음 패턴 선택
    void StartPattern(EBossPattern Type); // 패턴 시작
    void EndPattern();                    // 패턴 종료

    // 패턴별 실제 실행 함수
    void Pattern_Charge();   // 돌격
    void Pattern_DoubleSmash();  // 2회 내려찍기
    void Pattern_SparkWave();   // 전방 충격파
    void Pattern_JumpSmash();    // 점프 착지 충격
    void Pattern_SpinAttack();   // 회전 공격
    void Pattern_SparkWave2();   // 전방 충격파

    // ==============================================================
    // 페이즈 전환 체크
    // ==============================================================
    void CheckPhaseChange();

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 보스가 외부에서 피해를 받을 때 호출됨
    void ApplyDamage(float Amount);

    // ===============================
   // 공격 히트박스 컴포넌트
   // ===============================

   // 왼팔 내려찍기용 히트박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|HitBox")
    class UBoxComponent* LeftArmHitBox;

    // 오른팔 내려찍기용 히트박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|HitBox")
    UBoxComponent* RightArmHitBox;

    // 전방 충격파(스파크 웨이브) 판정용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|HitBox")
    USphereComponent* ShockWaveHitBox;

    // 회전 공격용 큰 히트박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|HitBox")
    USphereComponent* SpinAttackHitBox;

    // 점프 착지 충격파 판정용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|HitBox")
    UCapsuleComponent* LandingHitBox;

    // 히트박스 활성/비활성 함수
    UFUNCTION()
    void ActivateHitBox(FName HitBoxName);
    UFUNCTION()
    void DeactivateHitBox(FName HitBoxName);



    //보스 상태 전환 함수
    UFUNCTION()
    void SetBossState(EBossState NewState);

    // 보스 스킬
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* DoubleSmashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* ChargeMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* ChargeDashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* SparkWaveMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* SpinAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* JumpSmashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* SparkWaveMontage2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
    UAnimMontage* StunnedMontage;

    void MoveTowardsPlayer(float DeltaTime);
    void CheckAttack();
    void StartAttack();
    void StopAttack();

    //회전 공격 관련 변수
    UPROPERTY()
    bool bIsSpinning = false;
    UPROPERTY()
    float RotationTime = 0.f;
    UPROPERTY()
    float RotationDuration = 6.f;
    float StartYaw = 0.f; // 회전 시작 시 Yaw값
    // 회전 공격용 Timer
    FTimerHandle SpinTimer;

    ////////////일반공격+스킬공격 교차 실행용 변수, 함수//////////////
    // 공격 쿨타임
    float NormalAttackCooldown = 2.0f;
    float SkillCooldown = 12.0f;

    // 쿨타임 타이머
    bool bCanNormalAttack = true;
    bool bCanUseSkill = true;

    FTimerHandle NormalAttackTimer;
    FTimerHandle SkillTimer;

    void ResetNormalAttack();
    void ResetSkill();
    void TryAttack();
    void UseNormalAttack();
    void UseSkillAttack();


    //////// JumpShock 패턴용 변수 ////////
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Skills")
    float JumpHeight = 500.f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsPreparingJump = false;

    UPROPERTY(BlueprintReadOnly)
    bool bIsJumping = false;


    ///////패턴 공격 후 쿨타임 설정용 변수//////
    UPROPERTY()
    bool bIsIdlePaused = false;  // 2초 동안 Idle 유지용

    //FTimerHandle IdlePauseTimer;


    UPROPERTY(VisibleAnywhere)
    UBossHealthComponent* HealthComp;

    UPROPERTY(VisibleAnywhere)
    UBossDamageComponent* DamageComp;

    UPROPERTY(VisibleAnywhere)
    UBossHitReactionComponent* HitReactionComp;

    UFUNCTION()
    void HitReactPlay();

    bool bIsChargeMoving = false;
    void StartChargeMove();
    void StopChargeMove();

    int32 Phase1PatternIndex = 0;
    int32 Phase2PatternIndex = 0;

    //페이즈체인지 끝
    void StopPhaseChange();

    //hitbox damage 할당
    //일반공격 왼손
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float LeftArmDamageNoramal = 10.f;
    //일반공격 오른손
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float RightArmDamageNormal = 10.f;
    //점프공격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float JumpShockDamage = 15.f;
    //스파크웨이브1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float SparkWaveDamage = 15.f;
    //회전공격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float SpinAttackDamage = 0.5f;
    //스파크웨이브2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float SparkWave2Damage = 25.f;
    //대쉬돌진 주먹
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float DashChargeDamage = 20.f;

    //오버랩 함수 선언
    UFUNCTION()
    void OnHitBoxOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    // Boss1 클래스 멤버에 추가
    // 스킬 후 후딜 지속시간(애니 길이에 맞게 에디터에서 조정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Skills")
    float SkillCooldownDuration = 1.5f;

    // 쿨다운 타이머 핸들
    FTimerHandle SkillCooldownHandle;

    /** 공격 후 강제 Idle 상태를 유지하는 타이머 핸들 */
    FTimerHandle IdlePauseTimer;

    /** Idle 상태 강제 유지 중인지 나타내는 플래그 *//////////위에 하나 더 있음
   // UPROPERTY(VisibleAnywhere, Category = "AI State")
   //bool bIsIdlePaused = false;

   /** 공격 후 일정 시간 동안 Idle 상태를 유지하게 하는 함수 */
   void StartIdlePause(float Duration);

   /** IdlePause 상태를 해제하고 일반 Idle/추적 로직으로 복귀 */
   void StopIdlePause();

   //stunned 상태 유지용 타이머
   FTimerHandle StunTimer;

   //stunned 상태 시작 함수
   void EnterStunned(float Duration);

   //stunned 상태 종료 함수
   void ExitStunned();
};
