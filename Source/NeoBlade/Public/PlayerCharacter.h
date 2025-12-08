// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

// 클래스 전방 선언
class UPlayerCombatComponent; // 플레이어 전투상태 FSM 
class UPlayerHealthComponent; // 플레이어 체력 관리
class UPlayerDamageComponent; // 플레이어 데미지 관리 
class UPlayerHitReactionComponent; //  플레이어  타격 반응 관리
class UTargetingComponent; // 타겟팅 시스템 

UCLASS()
class NEOBLADE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 카메라와 플레이어 사이의 거리를 유지 시켜주는 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* springArmComp;

	// 플레이어 기본 카메라
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* PlayerCamComp;

	// =====================
	//     입력 관련 변수
	// =====================

	// 플레이어 컨트롤러 맵핑
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputMappingContext* imc_Player;
	// 카메라 위아래 입력
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_LookUp;
	// 카메라 좌우 입력 
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_Turn;

	// 기본 이동 WASD
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_Move;

	// 점프 Spascebar
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_Jump;

	// 달리기 LShift
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_Run;

	// 약공격 Mouse Left
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_LightAttack;

	// 강공격 Mouse Right
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_HeavyAttack;

	// 적 에어본 E 키
	UPROPERTY(EditDefaultsOnly, Category = "input")
	class UInputAction* ia_Launch;




	// 걷기 속도
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float walkSpeed = 400;
	// 달리기 속도
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
	float runSpeed = 600;

	// 달리기 시작
	void StartRun();
	// 달리기 종료
	void StopRun();


	// 이동 방향
	FVector direction;




	// ====================
	//    입력 관련 함수
	// ====================

	// 상하 회전 입력 처리
	void LookUp(const struct FInputActionValue& inputValue);
	// 좌우 회전 입력 처리
	void Turn(const struct FInputActionValue& inputValue);
	// 플레이어 기본 이동 입력 처리
	void Move(const struct FInputActionValue& inputValue);
	// 점프 입력 이벤트 처리 함수
	void InputJump(const struct FInputActionValue& inputValue);

	// 약공격 입력 이벤트 처리 함수
	void Input_LightAttack(const struct FInputActionValue& inputValue);
	// 강공격 입력 이벤트 처리 함수
	void Input_HeavyAttack(const struct FInputActionValue& inputValue);

	// 에어본 입력 이벤트 처리 함수
	void Input_Launch(const struct FInputActionValue& inputValue);




	// ===================
	//  카타나 무기 장착 변수 
	// ===================
protected:
	// 블루프린트 무기 클래스 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> WeaponClass;

	// ChildActorComponent로 뷰포트에서도 보이게 할 무기
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UChildActorComponent* WeaponComponent;



	// =====================
	//     2단점프 구현
	// =====================
public:
	// 현재 점프 횟수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump")
	int32 JumpCount;

	// 최대 점프 횟수 (2면 2단 점프)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	int32 MaxJumpCount;

	// 2단 점프 중인지 (AnimBP로 넘길 변수)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump")
	bool bIsDoubleJumping;

	// 착지중이면 다른 행동 할 수 없음
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsLanding = false;

	// 착지 애니메이션 시간을 측정하기 위한 타이머
	FTimerHandle LandingTimerHandle;

	// 애니메이션 길이에 맞춘 딜레이(초)
	float LandingDelay = 0.80f;

public:

	// 2단 점프 이펙트 (없어도 됨)
	//void PlayDoubleJumpEffect();

	// 착지 시 자동 호출 (점프 카운트 초기화)
	virtual void Landed(const FHitResult& Hit) override;

	// 착지 종료 처리 함수 (Timer에서 호출)
	void EndLanding();

	// ===============================
	//   전투 상태 State FSM 컴포넌트
	// ===============================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerCombatComponent* CombatComponent;

	// =====================================
	//  플레이어 체력 & 데미지 & 피격 반응 관리 컴포넌트
	// =====================================

	// 플레이어 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerHealthComponent* HealthComp;
	// 플레이어 공통 데미지 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerDamageComponent* DamageComp;
	// 플레이어  피격 반응
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPlayerHitReactionComponent* HitReactionComp;


	// ====================================
	//  짧게* 회피 전달 / 길게* 여기서 달리기 실행
	// ====================================
	UPROPERTY()
	bool bShiftPressed;

	UPROPERTY()
	float ShiftPressTime;

	void InputShiftPressed(const FInputActionValue& Value);
	void InputShiftReleased(const FInputActionValue& Value);

	float DodgeThreshold = 0.20f; // 0.2초 이하면 회피


	// ============================================================
	// ★ 1) 타겟팅 컴포넌트 (적 감지 담당)
	//    - 컴포넌트를 BP나 C++에서 붙여놓고 사용
	// ============================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UTargetingComponent* TargetingComp;

	// ============================================================
	// ★ 2) 카메라 락온 상태 플래그
	//    - true  : 적을 향해 자동으로 카메라 회전
	//    - false : 평소처럼 플레이어가 직접 카메라 조작
	// ============================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bCameraLock = false;

	// ============================================================
	// ★ 3) 카메라가 타겟을 따라가도록 회전시키는 함수
	//    - Tick()에서 매프레임 호출
	// ============================================================
	void AutoFollowTarget(float DeltaTime);

};
