// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "PlayerCombatComponent.h"
#include "PlayerHealthComponent.h"
#include "PlayerDamageComponent.h"
#include "PlayerHitReactionComponent.h"
#include "TargetingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GuardShieldComponent.h"
#include "PlayerAnim.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;



    // 전투 FSM 관리 클래스 플레이어에게 부착
    CombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComponent"));
    // 체력 관리 클래스 플레이어에게 부착
    HealthComp = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("HealthComponent"));
    // 데미지 관리 클래스 플레이어에게 부착
    DamageComp = CreateDefaultSubobject<UPlayerDamageComponent>(TEXT("DamageComponent"));
    // 피격 반응 관리 클래스 플레이어에게 부착 
    HitReactionComp = CreateDefaultSubobject<UPlayerHitReactionComponent>(TEXT("HitReactionComponent"));
    // 타겟팅 시스템 클래스 플레이어에게 부착 
    TargetingComp = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));
    // 가드 쉴드 시스템 클래스 플레이어에게 부착
    GuardShieldComp = CreateDefaultSubobject<UGuardShieldComponent>(TEXT("GuardShieldComponent"));



    // 스켈레탈 메시 데이터를 불러옴
    ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonTwinblast/Characters/Heroes/TwinBlast/Skins/Tier2/ShadowOps/Meshes/TwinBlast_ShadowOps.TwinBlast_ShadowOps'"));
    if (TempMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(TempMesh.Object);
        // 스켈레탈 메시의 위치를 설정
        GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));

        // 플레이어에게 카메라를 컨트롤 하기위한 컴포넌트를 붙인다.
        springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
        springArmComp->SetupAttachment(RootComponent); // 자기 자신을 루트로 지정
        springArmComp->SetRelativeLocation(FVector(0, 70, 90)); // 카메라의 위치
        springArmComp->TargetArmLength = 350.0; // 플레이어와 카메라의 거리
        springArmComp->bUsePawnControlRotation = true; // 회전 입력이 들어오면 스프링암 컴프를 회전 

        // 플레이어에게 카메라를 붙인다.
        PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamComp"));
        // 부모 설정
        PlayerCamComp->SetupAttachment(springArmComp);
        GetCharacterMovement()->bOrientRotationToMovement = true; // 플레이어의 앞 방향은 입력이 들어오는 방향 기준이다. 
        bUseControllerRotationYaw = false; // 카메라의 Yaw 방향과 플레이어의 이동방향을 통일 시키지 않는다.

        // ChildActorComponent 생성
        WeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponComponent"));

        // 캐릭터 Mesh에 붙일 소켓 지정
        WeaponComponent->SetupAttachment(GetMesh(), FName("Katana"));

        // 에디터에서 지정한 블루프린트 무기 클래스 사용
        WeaponComponent->SetChildActorClass(WeaponClass);

        // 최대 점프는 2단
        MaxJumpCount = 2;
        // 점프 카운트 시작은 0
        JumpCount = 0;
        // 더블 점프는 하고 있지 않음
        bIsDoubleJumping = false;

    }





}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[Player] BeginPlay HealthComp = %s"),
        HealthComp ? TEXT("VALID") : TEXT("NULL"));


    // 초기 속도를 걷기로 설정
    GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

    // 인핸스트 인풋 시스템을 사용하도록 함
    auto pc = Cast<APlayerController>(Controller);
    if (pc)
    {
        auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
        if (subsystem)
        {
            subsystem->AddMappingContext(imc_Player, 0);
        }
    }

    if (HealthComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Binding PlayerDeath delegate..."));

        HealthComp->OnPlayerDeath.AddDynamic(this, &APlayerCharacter::HandlePlayerDeath);

        if (CombatComponent)
        {
            HealthComp->OnPlayerDeath.AddDynamic(CombatComponent, &UPlayerCombatComponent::OnOwnerDeath);
        }
    }

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 플레이어 이동 처리
    direction = FTransform(GetControlRotation()).TransformVector(direction);
    AddMovementInput(direction);
    direction = FVector::ZeroVector;

    // 짧게 입력했는지 길게 입력했는지 확인하기 위함 shift
    if (bShiftPressed)
    {
        float HeldTime = GetWorld()->GetTimeSeconds() - ShiftPressTime;

        if (HeldTime > DodgeThreshold)
        {
            StartRun();
        }

    }





    // ============================================================
   // ★ 2) 카메라 자동 회전 조건
   //    - bCameraLock      : 타겟을 감지해서 "카메라 락온 모드"가 켜져 있을 때
   //    - TargetingComp    : 타겟팅 컴포넌트가 존재하고
   //    - CurrentTarget    : 실제로 타겟이 있을 때
   // ============================================================
    // 수정 버전:
    if (bCameraLock && TargetingComp && TargetingComp->CurrentTarget)
    {
        AutoFollowTarget(DeltaTime);
    }
    else
    {
        // ?? 락온 해제 → 카메라를 다시 플레이어 입력에 맡김
        GetCharacterMovement()->bOrientRotationToMovement = true;
        bUseControllerRotationYaw = false;
    }



}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    if (PlayerInput)
    {
        // 카메라 상하 , 좌우 
        PlayerInput->BindAction(ia_Turn, ETriggerEvent::Triggered, this, &APlayerCharacter::Turn);
        PlayerInput->BindAction(ia_LookUp, ETriggerEvent::Triggered, this, &APlayerCharacter::LookUp);
        // 기본 이동
        PlayerInput->BindAction(ia_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
        // 점프
        PlayerInput->BindAction(ia_Jump, ETriggerEvent::Started, this, &APlayerCharacter::InputJump);
        // 달리기 & 회피
        PlayerInput->BindAction(ia_Run, ETriggerEvent::Started, this, &APlayerCharacter::InputShiftPressed);
        PlayerInput->BindAction(ia_Run, ETriggerEvent::Completed, this, &APlayerCharacter::InputShiftReleased);

        // 약공격
        PlayerInput->BindAction(ia_LightAttack, ETriggerEvent::Started, this, &APlayerCharacter::Input_LightAttack);
        // 강공격
        PlayerInput->BindAction(ia_HeavyAttack, ETriggerEvent::Started, this, &APlayerCharacter::Input_HeavyAttack);
        // 에어본
        PlayerInput->BindAction(ia_Launch, ETriggerEvent::Started, this, &APlayerCharacter::Input_Launch);

        // 패링 / 가드
        PlayerInput->BindAction(ia_Parry_Guard, ETriggerEvent::Started, this, &APlayerCharacter::InputGuardPressed);
        PlayerInput->BindAction(ia_Parry_Guard, ETriggerEvent::Completed, this, &APlayerCharacter::InputGuardReleased);



    }

}

void APlayerCharacter::LookUp(const FInputActionValue& inputValue)
{

    float value = inputValue.Get<float>();
    AddControllerPitchInput(value);
}

void APlayerCharacter::Turn(const FInputActionValue& inputValue)
{
    float value = inputValue.Get<float>();
    AddControllerYawInput(value);

}


void APlayerCharacter::Move(const struct FInputActionValue& inputValue)
{
    // 착지 중이면 이동금지
    if (bIsLanding)
        return;

    // ★★★ 가드 중 이동 금지 ★★★
    if (CombatComponent && CombatComponent->IsGuarding())
    {
        return; // 이동 입력 무시
    }


    FVector2D value = inputValue.Get<FVector2D>();
    // 상하 입력 이벤트 처리
    direction.X = value.X;
    // 좌우 입력 이벤트 처리
    direction.Y = value.Y;

}

void APlayerCharacter::InputJump(const FInputActionValue& inputValue)
{
    // ==== = 착지 애니메이션 중 점프 금지 ==== =
    if (bIsLanding)
    {
        UE_LOG(LogTemp, Warning, TEXT("Jump Blocked: Landing"));
        return;
    }

    // ===== 공중 공격 / 지상 공격 / 회피 중 점프 금지 =====
    if (CombatComponent && (CombatComponent->IsAirAttacking() ||
        CombatComponent->IsInAttackState() ||
        CombatComponent->IsDodging()))
        return;

    // ★★★ 피격(HitStun) 중 점프 금지 ★★★
    if (CombatComponent && CombatComponent->CombatState == ECombatState::HitStun)
    {
        UE_LOG(LogTemp, Warning, TEXT("Jump Blocked: HitStun"));
        return;
    }


    // === 1단 점프 ===
    if (JumpCount == 0)
    {
        JumpCount = 1;
        Jump();
        return;
    }

    // === 2단 점프 ===
    if (JumpCount == 1)
    {
        JumpCount = 2;
        LaunchCharacter(FVector(0, 0, 600.f), false, true);
        return;
    }

    // JumpCount >= 2 → 무시
}


void APlayerCharacter::Input_LightAttack(const FInputActionValue& inputValue)
{
    UE_LOG(LogTemp, Warning, TEXT("Player InputLightAttack "));

    if (!CombatComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("CombatComponent NULL"));
        return;
    }

    // ★ SHIFT → 대쉬 공격
    if (bShiftPressed)
    {
        CombatComponent->InputAttack(EAttackType::DashLight);
        return;
    }

    // ★★ 공중 공격이면 AirLight 로 변환 ★★
    if (GetCharacterMovement()->IsFalling())
    {
        CombatComponent->InputAttack(EAttackType::AirLight);
        return;
    }

    // ★ 기본 지상 약공격
    CombatComponent->InputAttack(EAttackType::Light);
}


void APlayerCharacter::Input_HeavyAttack(const FInputActionValue& inputValue)
{
    if (!CombatComponent) return;

    // ★ SHIFT → 대쉬 공격
    if (bShiftPressed)
    {
        CombatComponent->InputAttack(EAttackType::DashHeavy);
        return;
    }

    // ★★ 공중 공격이면 AirHeavy 로 변환 ★★
    if (GetCharacterMovement()->IsFalling())
    {
        CombatComponent->InputAttack(EAttackType::AirHeavy);
        return;
    }

    // ★ 기본 지상 강공격
    CombatComponent->InputAttack(EAttackType::Heavy);
}

void APlayerCharacter::Input_Launch(const FInputActionValue& inputValue)
{
    if (!CombatComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("CombatComponent is NULL"));
        return;
    }

    // 공격 중이면 사용 금지
    if (CombatComponent->IsInAttackState())
    {
        UE_LOG(LogTemp, Warning, TEXT("Launch Skill Blocked: Attacking"));
        return;
    }

    // 공중 공격 중이면 사용 금지
    if (CombatComponent->IsAirAttacking())
    {
        UE_LOG(LogTemp, Warning, TEXT("Launch Skill Blocked: AirAttack"));
        return;
    }

    // 회피 중에도 사용 금지
    if (CombatComponent->IsDodging())
    {
        UE_LOG(LogTemp, Warning, TEXT("Launch Skill Blocked: Dodging"));
        return;
    }

    // 에어본 스킬 가능 상태가 아니면 여기서 리턴 
    if (!CombatComponent->CanLaunch())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Input] Launch blocked by CanLaunch()"));
        return;
    }

    // 에에본 스킬 공격 시작
    CombatComponent->LaunchSkill();
}



void APlayerCharacter::StartRun()
{
    if (bIsLanding) return;   // 착지중이면 금지

    GetCharacterMovement()->MaxWalkSpeed = runSpeed;

    /*UE_LOG(LogTemp, Warning, TEXT("[Run] StartRun | Speed=%f"),
       GetCharacterMovement()->MaxWalkSpeed);*/
}

void APlayerCharacter::StopRun()
{
    // 착지 중이더라도 걷기 속도로 복귀는 허용 가능
    GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

    /*UE_LOG(LogTemp, Warning, TEXT("[Run] StopRun | Speed=%f"),
       GetCharacterMovement()->MaxWalkSpeed);*/
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // ===== 점프 카운트 정상 초기화 =====
    JumpCount = 0;
    bIsDoubleJumping = false;

    // ===== 착지 애니메이션 제어용 플래그 =====
    bIsLanding = true;

    // ★ 이동을 DisableMovement 하면 점프 시스템이 깨짐!
    // GetCharacterMovement()->DisableMovement();  // ? 삭제

    // 착지 지연 후 복구
    GetWorldTimerManager().SetTimer(
        LandingTimerHandle,
        this,
        &APlayerCharacter::EndLanding,
        LandingDelay,
        false
    );
}

void APlayerCharacter::EndLanding()
{
    bIsLanding = false;

    // ===== 정상적으로 이동 복귀 =====
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}



void APlayerCharacter::InputShiftPressed(const FInputActionValue& Value)
{
    bShiftPressed = true;
    ShiftPressTime = GetWorld()->GetTimeSeconds();
}

void APlayerCharacter::InputShiftReleased(const FInputActionValue& Value)
{
    if (!bShiftPressed) return;
    bShiftPressed = false;

    float HeldTime = GetWorld()->GetTimeSeconds() - ShiftPressTime;

    if (HeldTime <= DodgeThreshold)
    {
        // 짧게 입력 → 회피 요청
        if (CombatComponent)
        {
            CombatComponent->RequestDodge();
        }
    }
    else
    {
        // 길게 입력 → 달리기 종료
        StopRun();
    }
}

// ============================================================
// ★ 3) 카메라를 타겟 방향으로 부드럽게 회전시키는 함수
//    - 실제 "카메라 회전"은 여기서만 한다
//    - TargetingComponent / CombatComponent 는 단지
//      bCameraLock On/Off만 조절하는 역할
// ============================================================
void APlayerCharacter::AutoFollowTarget(float DeltaTime)
{
    if (!TargetingComp || !TargetingComp->CurrentTarget || !Controller)
        return;

    // (1) 타겟까지의 방향 벡터 (Yaw만 필요하므로 Z=0)
    FVector ToTarget = TargetingComp->CurrentTarget->GetActorLocation() - GetActorLocation();
    ToTarget.Z = 0.f;

    if (ToTarget.IsNearlyZero())
        return;

    // (2) 원하는 회전값 (타겟을 바라보는 방향)
    FRotator DesiredRot = ToTarget.Rotation();

    // (3) 현재 카메라 회전 값
    FRotator CurrentControlRot = Controller->GetControlRotation();

    // (4) 부드러운 회전 보간 (너무 확 돌아가면 멀미 나니까)
    float CameraRotateSpeed = 7.f; // 숫자 크면 더 빠르게 타겟을 따라감

    FRotator SmoothedRot = FMath::RInterpTo(
        CurrentControlRot,
        DesiredRot,
        DeltaTime,
        CameraRotateSpeed
    );

    // (5) 최종 카메라 회전 적용
    Controller->SetControlRotation(SmoothedRot);
}

void APlayerCharacter::InputGuardPressed(const FInputActionValue& Value)
{
    bGuardPressed = true;

    if (CombatComponent)
    {
        CombatComponent->StartGuard();
    }
}


void APlayerCharacter::InputGuardReleased(const FInputActionValue& Value)
{
    bGuardPressed = false;

    if (CombatComponent)
    {
        CombatComponent->EndGuard();
    }
}

void APlayerCharacter::HandlePlayerDeath()
{
    UE_LOG(LogTemp, Error, TEXT("[Player] HandlePlayerDeath() CALLED!!!!"));

    // 1) 입력 차단
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] DisableInput OK"));
        DisableInput(PC);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Player] DisableInput FAILED (No PC found)"));
    }

    // 2) 이동 금지
    if (auto Move = GetCharacterMovement())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] Movement disabled"));
        Move->DisableMovement();
    }

    // 3) 죽음 애니메이션
    auto* Anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
    if (Anim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] AnimInstance OK"));

        if (Anim->DeathMontage)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Player] Playing Death Montage"));
            Anim->PlayDieMontage();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Player] DeathMontage is NULL!!!!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Player] AnimInstance is NULL or not UPlayerAnim!!!!"));
    }
}
