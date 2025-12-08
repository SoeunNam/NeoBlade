// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackType.h"
#include "PlayerCombatComponent.generated.h"

class APlayerCharacter;
class UPlayerAnim;


// ==============================
// Combat State
// ==============================
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    None        UMETA(DisplayName = "None"),
    Attack      UMETA(DisplayName = "Attack"),
    AirAttack   UMETA(DisplayName = "AirAttack"),
    Dodge       UMETA(DisplayName = "Dodge"),
    Parry       UMETA(DisplayName = "Parry"),
    HitStun     UMETA(DisplayName = "HitStun"),
    KnockDown   UMETA(DisplayName = "KnockDown"),
    Skill       UMETA(DisplayName = "Skill"),
    Dead        UMETA(DisplayName = "Dead")
};

// ==============================
// Attack Type 전역게서 공격 타입을 확인 할 수 있게 따로 클래스에서 관리
// ==============================
//UENUM(BlueprintType)
//enum class EAttackType : uint8
//{
//    None        UMETA(DisplayName = "None"),
//    Light       UMETA(DisplayName = "LightAttack"),
//    Heavy       UMETA(DisplayName = "HeavyAttack"),
//    AirLight    UMETA(DisplayName = "AirLightAttack"),
//    AirHeavy    UMETA(DisplayName = "AirHeavyAttack"),
//    Launch      UMETA(DisplayName = "Launch"),
//    DashLight   UMETA(DisplayName = "DashLightAttack"),
//    DashHeavy   UMETA(DisplayName = "DashHeavyAttack")
//};

// ==============================
// Weapon State
// ==============================
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Blade       UMETA(DisplayName = "Blade"),
    SpecialMode UMETA(DisplayName = "SpecialMode")
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UPlayerCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UPlayerCombatComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ============================================
// 캐릭터 / 애님 캐시
// ============================================
    UPROPERTY()
    APlayerCharacter* OwnerCharacter;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // ============================================
    // Combat FSM
    // ============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ECombatState CombatState = ECombatState::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ECombatState PrevCombatState = ECombatState::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EAttackType AttackType = EAttackType::None;

    void ChangeCombatState(ECombatState NewState);
    void OnCombatStateChanged(ECombatState OldState, ECombatState NewState);

    // ============================================
    // 배열 기반 콤보 시스템  핵심
    // ============================================
public:

    // Light = 5 / Heavy = 4 (조절 가능) -- 약 / 강공격
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> LightSections;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> HeavySections;
    // 대쉬 약공격
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> DashLightSections;
    // 대쉬 강공격
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> DashHeavySections;
    // 적을 공중에 띄운 상태
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> LaunchSections;
    // 공중 약공격
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> AirLightSections;
    // 공중 강공격
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
    TArray<FName> AirHeavySections;


    // 콤보 섞기용 (Light → Heavy → Light 지원)
    UPROPERTY()
    EAttackType QueuedAttackType = EAttackType::None;

    // Attack
    UFUNCTION()
    void InputAttack(EAttackType InputType);

    void StartAttack();
    void ContinueCombo();
    void EndAttack();

    // 현재 콤보 번호 (1 → Light1)
    int32 CurrentComboIndex = 0;

    // === 기존 MaxCombo는 삭제하거나 사용 안함 ===
    // int32 MaxCombo = 5;  // 배열 기반에서는 필요 없음

    bool bComboWindowOpen = false;
    bool bComboQueued = false;

    // 공격 중 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsAttacking = false;

    bool IsAttacking() const { return bIsAttacking; }

    // ============================================
    // 배열 기반 섹션/몽타주 관련 헬퍼 함수 선언
    // ============================================
protected:

    // 타입별 몽타주 반환
    UAnimMontage* GetMontageForType(EAttackType Type, UPlayerAnim* PlayerAnim) const;

    // 타입 + 콤보 인덱스로 섹션 이름 반환
    FName GetSectionNameForIndex(EAttackType Type, int32 ComboIndex) const;

    // 타입별 최대 콤보 수 반환 (배열 길이)
    int32 GetMaxComboForType(EAttackType Type) const;

public:
    // 애님노티파이에서 콤보창 열기/닫기
    void OpenComboWindow();
    void CloseComboWindow();

    // ============================================
    // 타격 판정 시스템
    // ============================================
public:
    void StartAttackHit(float Range, float Radius, float Damage);
    void EndAttackHit();
    void TickHitCheck();

    bool bHitActive = false;
    float HitRange;
    float HitRadius;
    float HitDamage;

    TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;


    // ==============================
    // 회피(Dodge) 시스템
    // ==============================
public:

    // PlayerCharacter 가 호출
    void RequestDodge();

    // Combat FSM 내부에서 처리되는 함수
    void StartDodge();

    // AnimNotify_DodgeEnd 가 호출해주는 함수
    void EndDodge();

    // 현재 회피 중인가?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsDodging = false;

    // 무적 시간(EndDodge에서 해제할 때 사용할 수도 있음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
    float DodgeInvincibleTime = 0.35f;

    // 무적 플래그 (데미지 컴포넌트에서 참고)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
    bool bInvincible = false;


    // Hit Stun 상태 종료 * 노티파이에서 호출함 HitStunEnd
    UFUNCTION()
    void EndHitStun();


    // ==============================
    // 공중 공격
    // ==============================
    bool bIsAirAttacking = false;
    float SavedGravityScale = 1.0f;

    void StartAirAttack();
    void ContinueAirCombo();
    void EndAirAttack();
    //void OnLandedDuringAirAttack();

    UFUNCTION()
    void AnimNotify_AirAttackEnd();

    // 공중 콤보 노티파이
    void OpenAirComboWindow();
    void CloseAirComboWindow();

    void StartAirAttackHit(float Range, float Radius, float Damage);
    void EndAirAttackHit();

    // ==============================
    // 상태/공격 타입 Query 함수들 (추가)
    // ==============================
public:

    // 현재 "전투 관련 상태"인지 (공격/공중공격/회피/피격/스킬 등)
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsInCombatState() const;

    // 현재 AttackState 인지 (지상 공격 or 공중 공격 포함)
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsInAttackState() const;

    // AttackType 기준 쿼리들
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsGroundAttackType() const;   // 지상 약/강/런치

    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsAirAttackType() const;      // 공중 약/강

    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsDashAttackType() const;     // 대쉬 약/강

    // "지금 공중 공격 중인가?" (상태 + 타입 + 플래그 종합)
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsAirAttacking() const;

    // 회피 중인가 ?
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool IsDodging() const;

    // 피격 가능한지?
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool CanBeHit() const;

    // 움직일 수 있는지?
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool CanMove() const;

    // 입력 가능한 상태?
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool CanReceiveInput() const;

    // 점프 가능한 상태?
    UFUNCTION(BlueprintPure, Category = "Combat|Query")
    bool CanJump() const;

    // 에어본 공격 스킬 사용 가능한 상태 ?
    bool CanLaunch() const;




    // ============================
    // ?? Launch 스킬 (E 키)
    // ============================
public:

    /**
     * E 키 입력 시 호출할 함수.
     * LaunchAttackMontage 를 재생하고,
     * 플레이어를 살짝 공중으로 띄운다.
     */
    void LaunchSkill();

    /**
     * Launch 몽타주가 끝나는 시점에 호출.
     * CombatState 를 None 으로 돌려서,
     * 이후 공중에서 공격 입력 시 AirAttack 으로 진입할 수 있게 한다.
     */
    void EndLaunchSkill();

    // --- Launch Cooldown ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Launch")
    float LaunchCooldown = 5.0f;   // 5초 쿨타임 기본값

    bool bLaunchOffCooldown = true;   // true = 사용 가능

    FTimerHandle LaunchCooldownHandle;

    void StartLaunchCooldown();
    void ResetLaunchCooldown();

    // ============================================================
    // ★ 전투 이동 보정 (전투 중에만 작동하는 이동 로직)
    // ============================================================
    void HandleCombatMovement(float DeltaTime);

    // ==============================================
    // 버그 수정  / 공격 중에 피격 당하면 기존의 공격 상태 정리가 안되어 잔재가 발생했던 것
    //===============================================

    // 공격, 공중공격 등 모든 상태를 즉시 정리
    void ForceEndCombatState();
};



