#include "PlayerCombatComponent.h"
#include "PlayerCharacter.h"
#include "PlayerAnim.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DamageComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"
#include "AttackType.h"
#include "TargetingComponent.h"
#include "GuardShieldComponent.h"


// =============================================================
// 기본
// =============================================================
UPlayerCombatComponent::UPlayerCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 공격 콤보 배열 초기화
    // 
    // ★ 약공격(5타)
    LightSections = {
        FName("Light1"),
        FName("Light2"),
        FName("Light3"),
        FName("Light4"),
        FName("Light5")
    };

    // ★ 강공격(4타)
    HeavySections = {
        FName("Heavy1"),
        FName("Heavy2"),
        FName("Heavy3"),
        FName("Heavy4")
    };

    // 대쉬 공격 
    DashLightSections = { FName("DashLight1"), FName("DashLight2") };
    DashHeavySections = { FName("DashHeavy1"), FName("DashHeavy2") };



    // 공중 약공격 (Air Light)
    AirLightSections = {
        FName("AirLight1"),
        FName("AirLight2"),
        FName("AirLight3"),
        FName("AirLight4"),
        FName("AirLight5"),
        FName("AirLight6"),
        FName("AirLight7"),
    };

    // 공중 강공격 (Air Heavy)
    AirHeavySections = {
        FName("AirHeavy1"),
        FName("AirHeavy2"),
        FName("AirHeavy3"),
        FName("AirHeavy4"),
        FName("AirHeavy5"),
        FName("AirHeavy6"),
        FName("AirHeavy7")
    };

    // 적을 공중에 넉백 시키고 공격 // 아직 미구현.
    LaunchSections = {};

}

void UPlayerCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        UE_LOG(LogTemp, Warning, TEXT("[Combat] BeginPlay OK"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] BeginPlay FAILED: OwnerCharacter NULL"));
    }


}

void UPlayerCombatComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bHitActive || !OwnerCharacter) return;

    // 충돌시 
    TickHitCheck();            // 기존 Sphere 판정


    // ============================================================
   // ★ 매 프레임 전투 관련 이동/회전 보정
   // ============================================================
    HandleCombatMovement(DeltaTime);


    // 전투 상태 state 실시간 체크
    FString StateName = UEnum::GetValueAsString(CombatState);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            1,                 // ID (고정)
            0.0f,              // Duration (0 → 매프레임 업데이트)
            FColor::Yellow,
            FString::Printf(TEXT("CombatState: %s"), *StateName)
        );
    }

    // ------------------------------------
    // ★ 디버깅: 공중 공격인데 몽타주가 안 돌면 강제 종료
    // ------------------------------------
    if (CombatState == ECombatState::AirAttack)
    {
        if (!AnimInstance->IsAnyMontagePlaying())
        {
            UE_LOG(LogTemp, Error,
                TEXT("[AirDebug] ERROR: AirAttack State BUT NO Montage is Playing → Force EndAirAttack"));
            EndAirAttack();
        }


    }

    // ----------------------------------------
// ★ Guard 상태일 때 패링 시간 체크
// ----------------------------------------
    if (bIsGuarding && bCanParry)
    {
        float Now = GetWorld()->GetTimeSeconds();
        if (Now - GuardStartTime > ParryWindowTime)
        {
            bCanParry = false;   // 패링 불가로 전환
            UE_LOG(LogTemp, Warning, TEXT("[Parry] Parry Window CLOSED"));
        }
    }

    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
    if (!Move) return;

    // ★★★ 가드 중 이동 불가 ★★★
    if (CombatState == ECombatState::Guard)
    {
        Move->Velocity = FVector::ZeroVector;  // 완전 정지
        Move->DisableMovement();               // 이동 자체 금지
        return;  // 아래 전투 이동 로직 실행 금지
    }



}



// =============================================================
// 상태머신 변경
// =============================================================
void UPlayerCombatComponent::ChangeCombatState(ECombatState NewState)
{
    if (CombatState == NewState) return;

    PrevCombatState = CombatState;
    CombatState = NewState;

    OnCombatStateChanged(PrevCombatState, CombatState);
}

void UPlayerCombatComponent::OnCombatStateChanged(
    ECombatState OldState,
    ECombatState NewState)
{
    UE_LOG(LogTemp, Warning,
        TEXT("[Combat] State Changed: %d -> %d"),
        (int32)OldState, (int32)NewState);


    if (NewState == ECombatState::Attack)
    {
        StartAttack();
    }
    else if (NewState == ECombatState::HitStun)
    {
        // 피격 시: 공격/회피 몽타주 중단
        if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
        {
            AnimInstance->StopAllMontages(0.1f);

            // ★★★★★ 가장 중요한 코드 ★★★★★
            ForceEndCombatState();  // 공격, 공중공격 등 모든 상태를 즉시 정리
        }

        // 회피 무적 강제 해제
        if (APlayerCharacter* PC = OwnerCharacter)
        {
            if (UHealthComponent* HP = PC->FindComponentByClass<UHealthComponent>())
            {
                HP->bInvincible = false;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("[Combat] Enter HitStun"));
    }
}

// =============================================================
// 입력 처리 (Light / Heavy / Air / Dash / Launch 공통)
// =============================================================
void UPlayerCombatComponent::InputAttack(EAttackType InputType)
{
    // ★★★ 스킬 중에는 공격 입력 자체를 막아야 함 ★★★
    if (CombatState == ECombatState::Skill)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InputAttack] Blocked because Skill is active"));
        return;
    }

    // ★★★★★★★★점프 중 대쉬 공격 금지★★★★★★★★★★★★★
    if ((InputType == EAttackType::DashLight || InputType == EAttackType::DashHeavy))
    {
        if (OwnerCharacter && OwnerCharacter->GetCharacterMovement()->IsFalling())
        {
            UE_LOG(LogTemp, Warning, TEXT("Dash Attack Blocked: Jumping"));
            return;
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Combat] InputAttack | Type=%d State=%d Combo=%d Window=%d QueueType=%d"),
        (int32)InputType,
        (int32)CombatState,
        CurrentComboIndex,
        bComboWindowOpen,
        (int32)QueuedAttackType);

    AttackType = InputType;

    // 피격/회피/패링 중엔 공격 불가
    if (CombatState == ECombatState::HitStun ||
        CombatState == ECombatState::Dodge ||
        CombatState == ECombatState::Parry)
        return;

    // =========================
    // ★ 공중에서 첫 공격 입력됨
    // =========================

       // ★★★ 가장 먼저 OwnerCharacter NULL 체크 ★★★
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] OwnerCharacter is NULL in InputAttack"));
        return;
    }

    // ★ AnimInstance NULL 체크 ★
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] AnimInstance is NULL in InputAttack"));
        return;
    }



    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
    if (!Move)
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] MoveComponent is NULL"));
        return;
    }



    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] InputAttack FAILED: OwnerCharacter is NULL"));
        return;
    }


    // 공중 공격이면 JumpCount 초기화 (점프가 아니기 때문)
    if (AttackType == EAttackType::AirLight || AttackType == EAttackType::AirHeavy)
    {
        if (OwnerCharacter)
        {
            OwnerCharacter->JumpCount = 0;
            OwnerCharacter->bIsDoubleJumping = false;
        }
    }


    if (CombatState == ECombatState::None)
    {
        if (Move && Move->IsFalling())
        {
            // 지상 Light → AirLight / 지상 Heavy → AirHeavy 변환
            if (InputType == EAttackType::Light)
                AttackType = EAttackType::AirLight;
            else if (InputType == EAttackType::Heavy)
                AttackType = EAttackType::AirHeavy;

            ChangeCombatState(ECombatState::AirAttack);
            StartAirAttack();
            return;
        }

        // 지상 공격
        ChangeCombatState(ECombatState::Attack);
        return;
    }

    // 공격 중 → 콤보 처리
    if (CombatState == ECombatState::Attack)
    {
        // 다음 공격 타입 예약 (Light→Heavy 섞기)
        QueuedAttackType = InputType;

        if (bComboWindowOpen)
        {
            ContinueCombo();
        }
        else
        {
            bComboQueued = true;
        }
    }

    // =========================
    // ★ 공중 콤보 처리
    // =========================
    if (CombatState == ECombatState::AirAttack)
    {
        if (bComboWindowOpen)
            ContinueAirCombo();
        else
            bComboQueued = true;

        return;
    }

}

// =============================================================
// 애님 노티파이: 콤보창 열기/닫기
// =============================================================
void UPlayerCombatComponent::OpenComboWindow()
{
    bComboWindowOpen = true;

    // 미리 입력 큐가 있으면 즉시 실행
    if (bComboQueued && QueuedAttackType != EAttackType::None)
    {
        bComboQueued = false;
        ContinueCombo();
    }
}

void UPlayerCombatComponent::CloseComboWindow()
{
    bComboWindowOpen = false;
}

// =============================================================
// 타입별 몽타주 얻기
// =============================================================
UAnimMontage* UPlayerCombatComponent::GetMontageForType(EAttackType Type, UPlayerAnim* PlayerAnim) const
{
    if (!PlayerAnim) return nullptr;

    switch (Type)
    {
    case EAttackType::Light:  return PlayerAnim->LightAttackMontage;
    case EAttackType::Heavy:  return PlayerAnim->HeavyAttackMontage;
    case EAttackType::AirLight:  return PlayerAnim->AirLightAttackMontage;
    case EAttackType::AirHeavy:  return PlayerAnim->AirHeavyAttackMontage;
    case EAttackType::DashLight:   return PlayerAnim->DashLightAttackMontage;
    case EAttackType::DashHeavy:   return PlayerAnim->DashHeavyAttackMontage;
    case EAttackType::Launch: return PlayerAnim->LaunchAttackMontage;
    default: return nullptr;
    }
}

// =============================================================
// 타입 + 콤보 인덱스로 섹션 이름 얻기
// =============================================================
FName UPlayerCombatComponent::GetSectionNameForIndex(EAttackType Type, int32 ComboIndex) const
{
    int32 Idx = ComboIndex - 1;
    if (Idx < 0) return NAME_None;

    const TArray<FName>* Arr = nullptr;

    switch (Type)
    {
    case EAttackType::Light:  Arr = &LightSections; break;
    case EAttackType::Heavy:  Arr = &HeavySections; break;
    case EAttackType::AirLight:   Arr = &AirLightSections; break;
    case EAttackType::AirHeavy:   Arr = &AirHeavySections; break;
    case EAttackType::DashLight:   Arr = &DashLightSections;  break;
    case EAttackType::DashHeavy:   Arr = &DashHeavySections;  break;
    case EAttackType::Launch: Arr = &LaunchSections; break;
    }

    if (!Arr || !Arr->IsValidIndex(Idx))
        return NAME_None;

    return (*Arr)[Idx];
}

// =============================================================
// 타입별 최대 콤보 수
// =============================================================
int32 UPlayerCombatComponent::GetMaxComboForType(EAttackType Type) const
{
    switch (Type)
    {
    case EAttackType::Light:  return LightSections.Num();
    case EAttackType::Heavy:  return HeavySections.Num();
    case EAttackType::AirLight:  return AirLightSections.Num();
    case EAttackType::AirHeavy:  return AirHeavySections.Num();
    case EAttackType::DashLight:   return DashLightSections.Num();
    case EAttackType::DashHeavy:   return DashHeavySections.Num();
    case EAttackType::Launch: return LaunchSections.Num();
    default: return 0;
    }
}

// =============================================================
// 공격 시작
// =============================================================
void UPlayerCombatComponent::StartAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("[Combat] StartAttack() LightSections.Num = %d"), LightSections.Num());



    bIsAttacking = true;

    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
    if (!PlayerAnim) return;

    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
    if (!Montage) return;

    CurrentComboIndex = 1;
    bComboQueued = false;
    bComboWindowOpen = false;
    QueuedAttackType = EAttackType::None;

    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
    UE_LOG(LogTemp, Warning, TEXT("[Combat] First SectionName = %s"), *SectionName.ToString());

    if (SectionName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[Combat] SectionName is NONE! Check montage section names."));
        return;
    }

    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);



}

// =============================================================
// 콤보 이어가기
// =============================================================
void UPlayerCombatComponent::ContinueCombo()
{
    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
    if (!PlayerAnim) return;

    // 예약된 공격이 있으면 AttackType 교체
    if (QueuedAttackType != EAttackType::None)
        AttackType = QueuedAttackType;

    int32 MaxComboThisType = GetMaxComboForType(AttackType);
    if (MaxComboThisType <= 0)
    {
        EndAttack();
        return;
    }

    int32 PrevIndex = CurrentComboIndex;
    CurrentComboIndex++;

    if (CurrentComboIndex > MaxComboThisType)
    {
        EndAttack();
        return;
    }

    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
    if (!Montage)
    {
        EndAttack();
        return;
    }

    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
    if (SectionName.IsNone())
    {
        EndAttack();
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Combat] ContinueCombo | %d → %d  Type=%d  Section=%s"),
        PrevIndex,
        CurrentComboIndex,
        (int32)AttackType,
        *SectionName.ToString());

    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);

    bComboQueued = false;
    bComboWindowOpen = false;
    QueuedAttackType = EAttackType::None;
}

// =============================================================
// 공격 종료
// =============================================================
void UPlayerCombatComponent::EndAttack()
{
    bIsAttacking = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[Combat] EndAttack | FinalCombo=%d Type=%d"),
        CurrentComboIndex,
        (int32)AttackType);

    CombatState = ECombatState::None;
    AttackType = EAttackType::None;

    CurrentComboIndex = 0;
    bComboQueued = false;
    bComboWindowOpen = false;
    QueuedAttackType = EAttackType::None;

    if (AnimInstance)
        AnimInstance->Montage_Stop(0.15f);
}

// =============================================================
// 히트 시작/종료
// =============================================================
void UPlayerCombatComponent::StartAttackHit(float Range, float Radius, float Damage)
{
    bHitActive = true;
    HitRange = Range;
    HitRadius = Radius;
    HitDamage = Damage;
    AlreadyHitActors.Empty();


}

void UPlayerCombatComponent::EndAttackHit()
{
    bHitActive = false;
    AlreadyHitActors.Empty();
}

// =============================================================
// 히트 판정 Tick
// =============================================================
//void UPlayerCombatComponent::TickHitCheck()
//{
//    if (!bHitActive || !OwnerCharacter) return;
//
//    FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, 50);
//    FVector End = Start + OwnerCharacter->GetActorForwardVector() * HitRange;
//
//    DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.01f, 0, 1.5f);
//
//    FCollisionShape Sphere = FCollisionShape::MakeSphere(HitRadius);
//    TArray<FHitResult> Hits;
//
//    FCollisionQueryParams P;
//    P.AddIgnoredActor(OwnerCharacter);
//
//    bool bHit = GetWorld()->SweepMultiByChannel(
//        Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, P);
//
//    if (!bHit) return;
//
//    for (auto& Hit : Hits)
//    {
//        AActor* Target = Hit.GetActor();
//        if (!Target || AlreadyHitActors.Contains(Target)) continue;
//
//        AlreadyHitActors.Add(Target);
//
//        if (UDamageComponent* Dmg = Target->FindComponentByClass<UDamageComponent>())
//        {
//            FVector Dir = OwnerCharacter->GetActorForwardVector();
//            Dmg->ApplyDamage(HitDamage, OwnerCharacter, Hit.ImpactPoint, Dir);
//        }
//    }
//}

void UPlayerCombatComponent::TickHitCheck()
{
    if (!bHitActive || !OwnerCharacter) return;

    /*UE_LOG(LogTemp, Warning,
        TEXT("[HIT DEBUG] HitRange=%.1f Radius=%.1f Active=%d"),
        HitRange, HitRadius, bHitActive);*/


    // ============================
    // 1) 공격 판정 지점 계산
    // ============================
    FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, 50);
    FVector End = Start + OwnerCharacter->GetActorForwardVector() * HitRange;

    FVector Center = (Start + End) * 0.5f;
    float HalfHeight = HitRange * 0.5f;
    FQuat CapsuleRotation = OwnerCharacter->GetActorForwardVector().ToOrientationQuat();

    // ============================
    // 2) 디버그 시각화 (★ 완전 강화된 버전)
    // ============================

    // 시작 → 끝 방향 화살표
    DrawDebugDirectionalArrow(
        GetWorld(),
        Start,
        End,
        40.f,
        FColor::Yellow,
        false,
        0.1f,
        0,
        4.f
    );

    // 공격 범위 중심 캡슐 (초록색)
    DrawDebugCapsule(
        GetWorld(),
        Center,
        HalfHeight,
        HitRadius,
        CapsuleRotation,
        FColor::Green,
        false,
        0.1f,
        0,
        2.f
    );

    // Start / End 지점 구체 (파랑, 빨강)
    DrawDebugSphere(GetWorld(), Start, HitRadius, 16, FColor::Cyan, false, 0.1f);
    DrawDebugSphere(GetWorld(), End, HitRadius, 16, FColor::Red, false, 0.1f);

    // 라인(중앙 축) 강조
    DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.1f, 0, 3.f);

    // ============================
    // 3) Sweep (SphereTrace)
    // ============================
    FCollisionShape Sphere = FCollisionShape::MakeSphere(HitRadius);
    TArray<FHitResult> Hits;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

    if (!bHit) return;

    // ============================
    // 4) 충돌 처리 + 디버그
    // ============================
    for (auto& Hit : Hits)
    {
        // 피격을 당한 상대에 대한 정보 ( 내가 때려서 맞은 액터는 누구인지 체크 )
        AActor* Target = Hit.GetActor();
        if (!Target || AlreadyHitActors.Contains(Target)) continue;

        AlreadyHitActors.Add(Target);

        // ↓↓↓ 충돌 지점 빨간 대형 구체 (명확하게 보이도록 크게)
        DrawDebugSphere(
            GetWorld(),
            Hit.ImpactPoint,
            25.f,
            20,
            FColor::Red,
            false,
            0.2f
        );

        // 텍스트 표시
        DrawDebugString(
            GetWorld(),
            Hit.ImpactPoint + FVector(0, 0, 40),
            TEXT("HIT!"),
            nullptr,
            FColor::White,
            0.2f,
            false
        );

        // 데미지 처리  ( 위에서 찾은 타격 대상의 DamageCompeont를 가져온다 )
        if (UDamageComponent* Dmg = Target->FindComponentByClass<UDamageComponent>())
        {
            // 맞은 대상의 데미지 컴포넌트가 데미지를 얼마 입힐건지 , 데미지를 누가 입히는지 때리는 대상을 전달 ( 플레이어 )
            //Dmg->DealDamage(HitDamage, OwnerCharacter);

            Dmg->AttackDealDamage(HitDamage, OwnerCharacter, AttackType);

        }
    }
}

// 적 에어본 판정을 높이기 위해 만든 에어본 전용 히트 박스인데 효과가 없어 잠시 주석처리
//void UPlayerCombatComponent::TickAirborneHitCheck()
//{
//    if (!bHitActive || !OwnerCharacter) return;
//
//    FVector Start = OwnerCharacter->GetActorLocation() + FVector(0, 0, 60);
//    FVector End = Start + OwnerCharacter->GetActorForwardVector() * HitRange;
//
//    FVector BoxHalfSize = FVector(250.f, 250.f, 250.f);
//
//
//    TArray<FHitResult> Hits;
//
//    FCollisionQueryParams Params;
//    Params.AddIgnoredActor(OwnerCharacter);
//
//    bool bHit = GetWorld()->SweepMultiByChannel(
//        Hits,
//        Start,
//        End,
//        FQuat::Identity,               // Quaternion 문제 제거
//        ECC_Pawn,                      // ★ Sphere와 동일하게!
//        FCollisionShape::MakeBox(BoxHalfSize),
//        Params
//    );
//
//    // 박스 디버그 - 중심 맞게
//    FVector Center = (Start + End) * 0.5f;
//
//    DrawDebugBox(
//        GetWorld(),
//        Center,
//        BoxHalfSize,
//        FColor::Purple,
//        false,
//        0.05f,
//        0,
//        3.f
//    );
//
//    if (!bHit) return;
//
//    for (auto& Hit : Hits)
//    {
//        AActor* Target = Hit.GetActor();
//        if (!Target || AlreadyHitActors.Contains(Target)) continue;
//
//        AlreadyHitActors.Add(Target);
//
//        if (UDamageComponent* Dmg = Target->FindComponentByClass<UDamageComponent>())
//        {
//            Dmg->AttackDealDamage(HitDamage, OwnerCharacter, AttackType);
//        }
//    }
//}
//
//

void UPlayerCombatComponent::RequestDodge()
{
    // 이미 회피 중이면 무시
    if (IsDodging())
        return;

    // 공중 공격 / 지상 공격 중이면 회피 불가
    if (IsAirAttacking() || IsInAttackState())
        return;

    // ★★★ 점프 중 회피 금지 ★★★
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        UE_LOG(LogTemp, Warning, TEXT("Dodge Blocked: Jumping"));
        return;
    }

    // 회피 시작
    StartDodge();
}

void UPlayerCombatComponent::StartDodge()
{
    if (!OwnerCharacter || !AnimInstance)
        return;

    CombatState = ECombatState::Dodge;
    bIsDodging = true;

    if (AnimInstance->IsAnyMontagePlaying())
    {
        AnimInstance->StopAllMontages(0.1f);
    }

    if (UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance))
    {
        PlayerAnim->PlayDodgeMontage();
    }

    // =============================
    //  ★ HealthComponent에서 무적 ON
    // =============================
    if (UHealthComponent* HP = OwnerCharacter->FindComponentByClass<UHealthComponent>())
    {
        HP->bInvincible = true;
    }
}


void UPlayerCombatComponent::EndDodge()
{
    CombatState = ECombatState::None;
    bIsDodging = false;

    // =============================
    //  ★ HealthComponent에서 무적 OFF
    // =============================
    if (UHealthComponent* HP = OwnerCharacter->FindComponentByClass<UHealthComponent>())
    {
        HP->bInvincible = false;
    }
}

void UPlayerCombatComponent::EndHitStun()
{
    // HitStun 상태가 아니면 무시
    if (CombatState != ECombatState::HitStun)
        return;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] EndHitStun()"));

    // 상태 복구
    ChangeCombatState(ECombatState::None);
}

//void UPlayerCombatComponent::StartAirAttack()
//{
//    UE_LOG(LogTemp, Warning, TEXT("[Combat] StartAirAttack"));
//
//    
//    bIsAttacking = true;
//    bIsAirAttacking = true;
//
//    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
//    if (Move)
//    {
//        // ★ 중력 제거 → 공중에서 멈춘 상태로 공격 가능
//        SavedGravityScale = Move->GravityScale;
//        Move->GravityScale = 0.0f;
//        Move->Velocity = FVector::ZeroVector;
//    }
//
//    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
//    if (!PlayerAnim) return;
//
//    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
//    if (!Montage) return;
//
//    CurrentComboIndex = 1;
//    bComboWindowOpen = false;
//    bComboQueued = false;
//    QueuedAttackType = EAttackType::None;
//
//    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
//
//    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);
//
//    CombatState = ECombatState::AirAttack;
//}


//
//void UPlayerCombatComponent::ContinueAirCombo()
//{
//    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
//    if (!PlayerAnim) return;
//
//    // ★ 공중 콤보는 계속 중력 0 유지
//    if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
//    {
//        Move->GravityScale = 0.0f;
//        Move->Velocity = FVector::ZeroVector;
//    }
//
//    // 타입 설정
//    if (QueuedAttackType != EAttackType::None)
//        AttackType = QueuedAttackType;
//
//    int32 MaxCombo = GetMaxComboForType(AttackType);
//    CurrentComboIndex++;
//
//    if (CurrentComboIndex > MaxCombo)
//    {
//        EndAirAttack();
//        return;
//    }
//
//    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
//    if (!Montage)
//    {
//        EndAirAttack();
//        return;
//    }
//
//    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
//
//    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);
//
//    bComboQueued = false;
//    bComboWindowOpen = false;
//}
//
//void UPlayerCombatComponent::EndAirAttack()
//{
//    UE_LOG(LogTemp, Warning, TEXT("[Combat] EndAirAttack"));
//
//    bIsAttacking = false;
//    bIsAirAttacking = false;
//
//    // ★ 중력 원래 값으로 복구
//    if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
//    {
//        Move->GravityScale = SavedGravityScale;
//    }
//
//    CombatState = ECombatState::None;
//    AttackType = EAttackType::None;
//
//    CurrentComboIndex = 0;
//    bComboQueued = false;
//    bComboWindowOpen = false;
//
//    if (AnimInstance)
//        AnimInstance->Montage_Stop(0.1f);
//
//
//}

void UPlayerCombatComponent::StartAirAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("=== StartAirAttack() ==="));

    bIsAttacking = true;
    bIsAirAttacking = true;

    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
    if (Move)
    {
        SavedGravityScale = Move->GravityScale;
        Move->GravityScale = 0.0f;
        Move->Velocity = FVector::ZeroVector;

        UE_LOG(LogTemp, Warning,
            TEXT("[AirAttack] Gravity OFF (Saved=%f)"), SavedGravityScale);
    }

    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
    if (!PlayerAnim)
    {
        UE_LOG(LogTemp, Error, TEXT("[AirAttack] AnimInstance NULL"));
        return;
    }

    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
    if (!Montage)
    {
        UE_LOG(LogTemp, Error, TEXT("[AirAttack] Montage NULL"));
        return;
    }

    CurrentComboIndex = 1;
    bComboWindowOpen = false;
    bComboQueued = false;
    QueuedAttackType = EAttackType::None;

    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
    UE_LOG(LogTemp, Warning, TEXT("[AirAttack] Play Section: %s"), *SectionName.ToString());

    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);

    CombatState = ECombatState::AirAttack;
}

//void UPlayerCombatComponent::ContinueAirCombo()
//{
//    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
//    if (!PlayerAnim)
//    {
//        UE_LOG(LogTemp, Error, TEXT("[AirCombo] AnimInstance NULL"));
//        return;
//    }
//
//    // 중력 계속 제거
//    if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
//    {
//        Move->GravityScale = 0.0f;
//        Move->Velocity = FVector::ZeroVector;
//    }
//
//    // 타입 변경
//    if (QueuedAttackType != EAttackType::None)
//        AttackType = QueuedAttackType;
//
//    int32 MaxCombo = GetMaxComboForType(AttackType);
//    CurrentComboIndex++;
//
//    UE_LOG(LogTemp, Warning, TEXT("[AirCombo] Continue Combo → %d / %d"), CurrentComboIndex, MaxCombo);
//
//    if (CurrentComboIndex > MaxCombo)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("[AirCombo] Max Reached → EndAirAttack()"));
//        EndAirAttack();
//        return;
//    }
//
//    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);
//    if (!Montage)
//    {
//        UE_LOG(LogTemp, Error, TEXT("[AirCombo] Montage NULL → Force End"));
//        EndAirAttack();
//        return;
//    }
//
//    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);
//    UE_LOG(LogTemp, Warning, TEXT("[AirCombo] Play Section: %s"), *SectionName.ToString());
//
//    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);
//
//    bComboQueued = false;
//    bComboWindowOpen = false;
//
//    // --------------- ★ 안전장치: 몽타주가 즉시 꺼져있으면 문제임 → 종료
//    if (!AnimInstance->Montage_IsPlaying(Montage))
//    {
//        UE_LOG(LogTemp, Error,
//            TEXT("[AirCombo] ERROR: Montage is NOT playing after PlayAnim! → Force EndAirAttack()"));
//
//        EndAirAttack();
//        return;
//    }
//}

void UPlayerCombatComponent::ContinueAirCombo()
{
    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
    if (!PlayerAnim) return;

    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();

    // 타입 설정
    if (QueuedAttackType != EAttackType::None)
        AttackType = QueuedAttackType;

    int32 MaxCombo = GetMaxComboForType(AttackType);
    CurrentComboIndex++;

    if (CurrentComboIndex > MaxCombo)
    {
        EndAirAttack();
        return;
    }

    UAnimMontage* Montage = GetMontageForType(AttackType, PlayerAnim);

    FName SectionName = GetSectionNameForIndex(AttackType, CurrentComboIndex);

    // ? 마지막 콤보면 중력 복구
    if (CurrentComboIndex == MaxCombo)
    {
        if (Move)
        {
            Move->GravityScale = SavedGravityScale;
            UE_LOG(LogTemp, Warning, TEXT("[AirCombo] Last hit → Gravity Restored"));
        }
    }
    else
    {
        // 여전히 공중 콤보 유지
        if (Move)
        {
            Move->GravityScale = 0.0f;
            Move->Velocity = FVector::ZeroVector;
        }
    }

    PlayerAnim->PlayAnimMontageSafe(Montage, SectionName);

    bComboQueued = false;
    bComboWindowOpen = false;
}

void UPlayerCombatComponent::EndAirAttack()
{
    UE_LOG(LogTemp, Warning, TEXT("=== EndAirAttack() ==="));

    bIsAttacking = false;
    bIsAirAttacking = false;

    // 중력 원복
    if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
    {
        Move->GravityScale = SavedGravityScale;
        UE_LOG(LogTemp, Warning, TEXT("[AirAttack] Gravity RESTORED to %f"), SavedGravityScale);
    }

    CombatState = ECombatState::None;
    AttackType = EAttackType::None;

    CurrentComboIndex = 0;
    bComboQueued = false;
    bComboWindowOpen = false;

    /*if (AnimInstance)
    {
        AnimInstance->Montage_Stop(0.1f);
        UE_LOG(LogTemp, Warning, TEXT("[AirAttack] Montage STOP"));
    }*/
}



void UPlayerCombatComponent::OpenAirComboWindow()
{
    bComboWindowOpen = true;

    if (bComboQueued)
    {
        bComboQueued = false;
        ContinueAirCombo();
    }
}

void UPlayerCombatComponent::CloseAirComboWindow()
{
    bComboWindowOpen = false;
}

void UPlayerCombatComponent::StartAirAttackHit(float Range, float Radius, float Damage)
{
    StartAttackHit(Range, Radius, Damage);
}

void UPlayerCombatComponent::EndAirAttackHit()
{
    EndAttackHit();
}

// Finish 노티파이가 호출
void UPlayerCombatComponent::AnimNotify_AirAttackEnd()
{
    EndAirAttack();
}

// ==============================
// 상태/타입 Query 함수 구현
// ==============================

bool UPlayerCombatComponent::IsInCombatState() const
{
    // Dead 제외한, 전투 관련 상태면 true
    switch (CombatState)
    {
    case ECombatState::Attack:
    case ECombatState::AirAttack:
    case ECombatState::Dodge:
    case ECombatState::Parry:
    case ECombatState::HitStun:
    case ECombatState::KnockDown:
    case ECombatState::Skill:
        return true;
    default:
        return false;
    }
}

bool UPlayerCombatComponent::IsInAttackState() const
{
    return (CombatState == ECombatState::Attack ||
        CombatState == ECombatState::AirAttack);
}

bool UPlayerCombatComponent::IsGroundAttackType() const
{
    // 지상에서 사용하는 공격 타입들
    return (AttackType == EAttackType::Light ||
        AttackType == EAttackType::Heavy ||
        AttackType == EAttackType::Launch);
}

bool UPlayerCombatComponent::IsAirAttackType() const
{
    return (AttackType == EAttackType::AirLight ||
        AttackType == EAttackType::AirHeavy);
}

bool UPlayerCombatComponent::IsDashAttackType() const
{
    return (AttackType == EAttackType::DashLight ||
        AttackType == EAttackType::DashHeavy);
}

bool UPlayerCombatComponent::IsAirAttacking() const
{
    // CombatState, AttackType, 내부 플래그를 모두 종합해서 "공중 공격 중" 판정
    if (CombatState == ECombatState::AirAttack)
        return true;

    if (bIsAirAttacking)
        return true;

    if (IsAirAttackType())
        return true;

    return false;
}

bool UPlayerCombatComponent::IsDodging() const
{
    return CombatState == ECombatState::Dodge;
}

bool UPlayerCombatComponent::CanBeHit() const
{
    return !(CombatState == ECombatState::Dodge ||
        CombatState == ECombatState::Parry ||
        CombatState == ECombatState::KnockDown ||
        CombatState == ECombatState::Dead);
}

bool UPlayerCombatComponent::CanMove() const
{
    return CombatState != ECombatState::HitStun &&
        CombatState != ECombatState::KnockDown &&
        CombatState != ECombatState::Dead;
}

bool UPlayerCombatComponent::CanReceiveInput() const
{
    return CombatState != ECombatState::HitStun &&
        CombatState != ECombatState::Dead;
}

bool UPlayerCombatComponent::CanJump() const
{
    if (IsAirAttacking()) return false;
    if (IsDodging()) return false;
    if (IsInAttackState()) return false;

    return true;
}

bool UPlayerCombatComponent::CanLaunch() const
{
    // 캐릭터가 없으면 당연히 불가능
    if (!OwnerCharacter)
        return false;

    // 1) 쿨타임이 아직 안 돌았으면 사용 불가
    if (!bLaunchOffCooldown)
        return false;

    // 2) 이미 다른 전투 상태면 사용 불가
    //    (공격 / 공중공격 / 회피 / 패링 / 피격 / 다운 / 스킬 / 사망)
    switch (CombatState)
    {
    case ECombatState::Attack:
    case ECombatState::AirAttack:
    case ECombatState::Dodge:
    case ECombatState::Parry:
    case ECombatState::HitStun:
    case ECombatState::KnockDown:
    case ECombatState::Skill:
    case ECombatState::Dead:
        return false;
    default:
        break;
    }

    // 3) 이미 공중에 떠 있으면 Launch 금지
    //    (점프 중 / 떨어지는 중 / 다른 공중 스킬 도중 난사 방지)
    if (const UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
    {
        if (Move->IsFalling())
            return false;
    }

    // 위 조건 다 통과하면 사용 가능
    return true;
}


// ==============================
//  E 키 Launch 스킬
// ==============================
void UPlayerCombatComponent::LaunchSkill()
{
    // 기본 안전 체크
    if (!OwnerCharacter || !AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[Launch] LaunchSkill FAILED: Owner or AnimInstance is NULL"));
        return;
    }

    // 에어본 스킬을 쓸 수 있는 상태인지 체크
    if (!CanLaunch())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Launch] Cannot Launch in current State=%d"), (int32)CombatState);
        return;
    }

    // ★ 쿨타임 시작
    StartLaunchCooldown();

    // 상태 설정: 스킬 상태로 전환
    CombatState = ECombatState::Skill;
    AttackType = EAttackType::Launch;

    // 애니메이션 인스턴스 캐스팅
    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
    if (!PlayerAnim)
    {
        UE_LOG(LogTemp, Error, TEXT("[Launch] PlayerAnim is NULL"));
        return;
    }

    // Launch 전용 몽타주 가져오기
    UAnimMontage* LaunchMontage = PlayerAnim->LaunchAttackMontage;
    if (!LaunchMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("[Launch] LaunchAttackMontage is NULL (Check PlayerAnim)"));
        return;
    }

    // 섹션 이름 (Montage 안에 "Launch1" 같은 이름으로 섹션 만들어 둬야 함)
    FName SectionName = FName("Launch1");

    UE_LOG(LogTemp, Warning, TEXT("[Launch] LaunchSkill → Play Montage %s (Section=%s)"),
        *LaunchMontage->GetName(), *SectionName.ToString());

    // 몽타주 재생
    PlayerAnim->PlayAnimMontageSafe(LaunchMontage, SectionName);




    // ==============================
    //  실제로 캐릭터를 살짝 공중으로 띄우기
    //  (RootMotion으로 해결할 거면 이 부분은 빼도 됨)
    // ==============================
    if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
    {
        // 위로 살짝 튀어오르는 힘
        FVector LaunchVelocity = FVector(0.f, 0.f, 850.f);
        OwnerCharacter->LaunchCharacter(LaunchVelocity, false, false);

        //  이 한 줄이 없어서 공중 공격 연계가 안 됐던 것
        OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);

        UE_LOG(LogTemp, Warning,
            TEXT("[Launch] LaunchCharacter Z=%.1f"), LaunchVelocity.Z);

        // ★ Launch 후 곧바로 공중 공격 입력 허용
        if (Move->IsFalling())
        {
            // 지상 공격 입력을 공중 공격 타입으로 변환
            if (AttackType == EAttackType::Light)
                AttackType = EAttackType::AirLight;
            else if (AttackType == EAttackType::Heavy)
                AttackType = EAttackType::AirHeavy;
        }

        // // ==============================
        //// 타겟팅된 적에게 Launch 데미지 전달
        //// ==============================
        // UTargetingComponent* TargetComp = OwnerCharacter->FindComponentByClass<UTargetingComponent>();
        // if (TargetComp && TargetComp->CurrentTarget)
        // {
        //     AActor* Enemy = TargetComp->CurrentTarget;

        //     if (UDamageComponent* Dmg = Enemy->FindComponentByClass<UDamageComponent>())
        //     {
        //         // 데미지는 없어도 됨 (0f 가능)
        //         Dmg->AttackDealDamage(0.f, OwnerCharacter, EAttackType::Launch);

        //         UE_LOG(LogTemp, Warning,
        //             TEXT("[Launch] Send Launch Damage to %s"), *Enemy->GetName());
        //     }

        // }



    }


    //// ========================================
    //// ★★★★★ 지금 타겟팅 된 적만 띄우기 ★★★★★
    //// ========================================
    //UTargetingComponent* Target = OwnerCharacter->FindComponentByClass<UTargetingComponent>();
    //if (Target && Target->CurrentTarget)
    //{
    //    AActor* Enemy = Target->CurrentTarget;

    //    if (UDamageComponent* Dmg = Enemy->FindComponentByClass<UDamageComponent>())
    //    {
    //        // Launch 타입으로 강제 HitReaction 호출
    //        Dmg->AttackDealDamage(0.f, OwnerCharacter, EAttackType::Launch);
    //    }
    //}
}

// ==============================
//  Launch 몽타주 종료 시 호출
//  (AnimNotify에서 호출 예정)
// ==============================
void UPlayerCombatComponent::EndLaunchSkill()
{
    CombatState = ECombatState::None;
    AttackType = EAttackType::None;

    // ★ 공중 공격 준비를 위한 필수 설정
    if (OwnerCharacter)
    {
        OwnerCharacter->JumpCount = 0;
        OwnerCharacter->bIsDoubleJumping = false;

        UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
        if (Move)
        {
            Move->SetMovementMode(MOVE_Falling);
        }
    }
}

void UPlayerCombatComponent::StartLaunchCooldown()
{
    bLaunchOffCooldown = false;

    // LaunchCooldown 초 후에 ResetLaunchCooldown 호출
    GetWorld()->GetTimerManager().SetTimer(
        LaunchCooldownHandle,
        this,
        &UPlayerCombatComponent::ResetLaunchCooldown,
        LaunchCooldown,
        false
    );
}

void UPlayerCombatComponent::ResetLaunchCooldown()
{
    bLaunchOffCooldown = true;
}

void UPlayerCombatComponent::HandleCombatMovement(float DeltaTime)
{
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
    if (!Move) return;

    // ============================================================
    // 1) 타겟이 있을 때, 전투 중이면 플레이어 몸 방향도 부드럽게 타겟을 향하게 회전
    //    (카메라는 Player가 이미 돌려주고 있고, 여기는 "몸 회전" 담당)
    // ============================================================
    UTargetingComponent* TargetComp =
        OwnerCharacter->FindComponentByClass<UTargetingComponent>();

    if (TargetComp && TargetComp->CurrentTarget &&
        CombatState == ECombatState::Attack)
    {
        FVector ToTarget = TargetComp->CurrentTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
        ToTarget.Z = 0.f;

        if (!ToTarget.IsNearlyZero())
        {
            FRotator DesiredRot = ToTarget.Rotation();

            float RotateSpeed = 10.f; // 몸 회전 속도
            FRotator NewRot = FMath::RInterpTo(
                OwnerCharacter->GetActorRotation(),
                DesiredRot,
                DeltaTime,
                RotateSpeed
            );

            OwnerCharacter->SetActorRotation(NewRot);
        }


    }

    //// ============================================================
    //// 2) 근접 공격 중 전진 보정 (콤보 이어지게 약하게 앞으로 밀어주기)
    //// ============================================================
    //if (CombatState == ECombatState::Attack)
    //{
    //    FVector Forward = OwnerCharacter->GetActorForwardVector();
    //    // 값 0.05~0.15 사이 튜닝
    //    OwnerCharacter->AddMovementInput(Forward, 0.08f);
    //}

    //// ============================================================
    //// 3) 자동 어프로치: 적과 너무 애매한 거리일 때 알아서 붙기
    //// ============================================================
    //if (CombatState == ECombatState::Attack && TargetComp && TargetComp->CurrentTarget)
    //{
    //    float Dist = FVector::Dist(
    //        OwnerCharacter->GetActorLocation(),
    //        TargetComp->CurrentTarget->GetActorLocation());

    //    // 예: 120 ~ 300 사이 거리에 있으면 앞으로 당겨준다
    //    if (Dist > 120.f && Dist < 300.f)
    //    {
    //        FVector Forward = OwnerCharacter->GetActorForwardVector();
    //        OwnerCharacter->AddMovementInput(Forward, 0.4f);
    //    }
    //}

    // ============================================================
    // 4) 공중 전투일 때 높이 보정 (선택사항)
    //    - 에어본 적을 따라가려고 할 때 느낌 좋게 만들 수 있음
    // ============================================================
    if (CombatState == ECombatState::AirAttack && TargetComp && TargetComp->CurrentTarget)
    {
        FVector TargetLoc = TargetComp->CurrentTarget->GetActorLocation();
        FVector MyLoc = OwnerCharacter->GetActorLocation();

        float HeightDiff = TargetLoc.Z - MyLoc.Z;

        if (HeightDiff > 30.f)
        {
            // 타겟보다 많이 아래 → 위로 살짝 올려주기
            Move->AddForce(FVector(0, 0, 5000.f));
        }
        else if (HeightDiff < -30.f)
        {
            // 타겟보다 많이 위 → 살짝 내려주기
            Move->AddForce(FVector(0, 0, -3000.f));
        }
    }
}

void UPlayerCombatComponent::ForceEndCombatState()
{
    // 공격 중이면 종료
    if (IsInAttackState())
    {
        EndAttack();         // 지상 공격
        EndAirAttack();      // 공중 공격
    }

    // 공중공격 깨졌을 때 중력 복구
    if (bIsAirAttacking)
    {
        if (UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement())
        {
            Move->GravityScale = SavedGravityScale;
        }
        bIsAirAttacking = false;
    }

    // 콤보 변수 초기화
    CurrentComboIndex = 0;
    QueuedAttackType = EAttackType::None;
    bComboWindowOpen = false;
    bComboQueued = false;
    AttackType = EAttackType::None;

    // HitActive 강제 종료 (공격 판정 꺼야 함)
    bHitActive = false;
    AlreadyHitActors.Empty();


    // ======================================================
    // ★ 착지 중 피격 → 착지 강제 종료 (무한 점프 버그 해결)
    // ======================================================
    if (OwnerCharacter && OwnerCharacter->bIsLanding)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Landing interrupted by Hit → Force End Landing"));

        // 착지 상태 해제
        OwnerCharacter->bIsLanding = false;

        // 타이머 중지
        OwnerCharacter->GetWorldTimerManager().ClearTimer(OwnerCharacter->LandingTimerHandle);

        // 정상 이동 모드 복귀
        if (auto* Move = OwnerCharacter->GetCharacterMovement())
        {
            Move->SetMovementMode(EMovementMode::MOVE_Walking);
        }

        // 점프카운트 정상화
        OwnerCharacter->JumpCount = 0;
        OwnerCharacter->bIsDoubleJumping = false;
    }
}



//
//
//void UPlayerCombatComponent::InputParry()
//{
//    if (!OwnerCharacter || !AnimInstance)
//        return;
//
//    // 다른 상태에서는 패링 불가
//    if (CombatState != ECombatState::None)
//        return;
//
//    // 상태 전환
//    ChangeCombatState(ECombatState::Parry);
//
//    // 패링 몽타주 플레이
//    UPlayerAnim* PlayerAnim = Cast<UPlayerAnim>(AnimInstance);
//    if (!PlayerAnim || !PlayerAnim->ParryMontage)
//    {
//        UE_LOG(LogTemp, Error, TEXT("[Parry] ParryMontage is NULL"));
//        return;
//    }
//
//    OwnerCharacter->PlayAnimMontage(PlayerAnim->ParryMontage);
//
//    // 노티파이가 열어 줄 것이므로 false
//    bIsParryWindow = false;
//
//    UE_LOG(LogTemp, Warning, TEXT("[Parry] Start Parry"));
//}
//
//void UPlayerCombatComponent::BeginParryWindow()
//{
//    if (CombatState == ECombatState::Parry)
//    {
//        bIsParryWindow = true;
//        UE_LOG(LogTemp, Warning, TEXT("[Parry] Window START"));
//    }
//}
//
//void UPlayerCombatComponent::EndParryWindow()
//{
//    bIsParryWindow = false;
//    UE_LOG(LogTemp, Warning, TEXT("[Parry] Window END"));
//}
//
//void UPlayerCombatComponent::OnParrySuccess(AActor* Enemy)
//{
//    bIsParryWindow = false;
//
//    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), ParryGlobalTimeDilation);
//    OwnerCharacter->CustomTimeDilation = ParryPlayerTimeDilation;
//
//    // 적에게 패링 타입 데미지 전달
//    if (Enemy)
//    {
//        if (UDamageComponent* Dmg = Enemy->FindComponentByClass<UDamageComponent>())
//        {
//            // ★ 패링 타입 전달
//            Dmg->AttackDealDamage(0.f, OwnerCharacter, EAttackType::Parry);
//        }
//    }
//
//    // 슬로모션 종료 예약
//    FTimerHandle Timer;
//    GetWorld()->GetTimerManager().SetTimer(
//        Timer, this, &UPlayerCombatComponent::EndParrySlowMotion,
//        ParrySlowMotionDuration, false
//    );
//
//    UE_LOG(LogTemp, Warning, TEXT("[Parry] SUCCESS!"));
//}
//
//void UPlayerCombatComponent::EndParrySlowMotion()
//{
//    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
//    if (OwnerCharacter) OwnerCharacter->CustomTimeDilation = 1.0f;
//}
//
//void UPlayerCombatComponent::ForceEndParry()
//{
//    bIsParryWindow = false;
//
//    // 상태 정상화
//    ChangeCombatState(ECombatState::None);
//
//    // 혹시 남아있는 타임딜레이션 복구
//    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
//    if (OwnerCharacter)
//        OwnerCharacter->CustomTimeDilation = 1.0f;
//
//    UE_LOG(LogTemp, Warning, TEXT("[Parry] END Parry"));
//}

void UPlayerCombatComponent::StartGuard()
{
    // ★ 가장 먼저 OwnerCharacter / AnimInstance 안정성 체크
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[Guard] StartGuard FAILED: OwnerCharacter is NULL"));
        return;
    }

    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[Guard] StartGuard FAILED: AnimInstance is NULL"));
        return;
    }

    // ★★★ 공중에서는 가드 불가능 ★★★
    if (OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Guard] Blocked: Cannot Guard in Air"));
        return;
    }


    if (bIsGuarding)
        return;

    bIsGuarding = true;
    bCanParry = true;
    GuardStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    ChangeCombatState(ECombatState::Guard);


    // ★ 캐릭터가 카메라(타겟) 방향을 강제로 바라보도록 설정
    OwnerCharacter->bUseControllerRotationYaw = true;

    // 애님 플래그
    if (auto* Anim = Cast<UPlayerAnim>(AnimInstance))
    {
        Anim->bIsGuarding = true;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Guard] START"));

    // =============================
    // ★ 쉴드 컴포넌트 연동
    // =============================
    if (auto* Shield = OwnerCharacter->FindComponentByClass<UGuardShieldComponent>())
    {
        if (!Shield->CanActivateShield())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Guard] Shield not ready (cooldown etc)"));
            return;
        }

        Shield->ActivateShield();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Guard] No GuardShieldComponent on Player"));
    }
}

void UPlayerCombatComponent::EndGuard()
{
    bIsGuarding = false;
    bCanParry = false;

    ChangeCombatState(ECombatState::None);

    if (auto* Anim = Cast<UPlayerAnim>(AnimInstance))
    {
        Anim->bIsGuarding = false;
    }

    // ★ 이동 복구
    if (auto* Move = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr)
    {
        Move->SetMovementMode(EMovementMode::MOVE_Walking);
    }

    // ★ 회전 방식 원래대로 복구
    OwnerCharacter->bUseControllerRotationYaw = false;

    // ★ GuardShield 비활성화까지 해 주는 게 자연스러움
    if (OwnerCharacter)
    {
        if (auto* Shield = OwnerCharacter->FindComponentByClass<UGuardShieldComponent>())
        {
            Shield->DeactivateShield();   // ← 이 함수 GuardShield 쪽에 하나 만들어두면 깔끔
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Guard] END"));
}


// =========================================================
// DamageComponent → Parry 시도
// =========================================================
//bool UPlayerCombatComponent::TryParry(AActor* Attacker)
//{
//    if (!bIsGuarding || !bCanParry)
//        return false;
//
//    float Now = GetWorld()->GetTimeSeconds();
//    if ((Now - GuardStartTime) > ParryWindowTime)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("[Parry] Window CLOSED"));
//        return false;
//    }
//
//    // ★ 패링 성공 ★
//    bCanParry = false;
//    ChangeCombatState(ECombatState::Parry);
//
//    UE_LOG(LogTemp, Warning, TEXT("[Parry] SUCCESS"));
//
//    // 1) 적 스턴
//    if (Attacker)
//    {
//        if (auto* HitComp = Attacker->FindComponentByClass<UHitReactionComponent>())
//            HitComp->EnterParryStun();
//    }
//
//    // 2) 패링 성공 모션 재생
//    if (auto* Anim = Cast<UPlayerAnim>(AnimInstance))
//    {
//        if (Anim->ParrySuccessMontage)
//        {
//            OwnerCharacter->PlayAnimMontage(Anim->ParrySuccessMontage);
//            UE_LOG(LogTemp, Warning, TEXT("[Parry] Play ParrySuccessMontage"));
//        }
//    }
//
//    return true;
//}

bool UPlayerCombatComponent::TryParry(AActor* Attacker)
{
    if (!bIsGuarding || !bCanParry)
        return false;

    float Now = GetWorld()->GetTimeSeconds();
    if ((Now - GuardStartTime) > ParryWindowTime)
        return false;

    // 패링 성공
    bCanParry = false;
    ChangeCombatState(ECombatState::Parry);

    UE_LOG(LogTemp, Warning, TEXT("[Parry] SUCCESS"));

    // -------------------------------------------------------------
    // ★ 패링 슬로우 모션 시작 ★
    // -------------------------------------------------------------
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.15f); // 전체 슬로우
    OwnerCharacter->CustomTimeDilation = 0.6f;                  // 플레이어는 덜 느림

    // 일정 시간 후 복구
    GetWorld()->GetTimerManager().SetTimer(
        ParrySlowTimer,
        this,
        &UPlayerCombatComponent::EndParrySlowMotion,
        ParrySlowMotionDuration,    // 보통 0.2~0.35초 정도
        false
    );

    if (Attacker)
    {
        if (auto* Dmg = Attacker->FindComponentByClass<UDamageComponent>())
        {
            // 패링 타입으로 공격 전달 (데미지는 0)
            Dmg->AttackDealDamage(0.f, OwnerCharacter, EAttackType::Parry);
        }
    }


    // 패링 성공 애니메이션
    if (auto* Anim = Cast<UPlayerAnim>(AnimInstance))
    {
        if (Anim->ParrySuccessMontage)
            OwnerCharacter->PlayAnimMontage(Anim->ParrySuccessMontage);
    }

    return true;
}


void UPlayerCombatComponent::RequestParry()
{
    // ? 이제 사용하지 않음
    UE_LOG(LogTemp, Warning, TEXT("[Parry] RequestParry() is disabled in new guard system"));
}

void UPlayerCombatComponent::EndParrySlowMotion()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f); // 전체 속도 정상화
    OwnerCharacter->CustomTimeDilation = 1.0f;                  // 플레이어 속도 복구
}

void UPlayerCombatComponent::OnOwnerDeath()
{
    UE_LOG(LogTemp, Warning, TEXT("[Combat] Owner DIED → CombatState = Dead"));

    // 상태 강제 종료
    ForceEndCombatState();

    // 이동, 공격, 피격 등 전부 봉인
    CombatState = ECombatState::Dead;

    // 이동 완전 정지
    if (OwnerCharacter)
    {
        OwnerCharacter->GetCharacterMovement()->DisableMovement();
    }

    // 공격/회피 판정 OFF
    bHitActive = false;
    bIsAttacking = false;
    bIsAirAttacking = false;
    bIsDodging = false;
    bIsGuarding = false;

    AlreadyHitActors.Empty();
}
