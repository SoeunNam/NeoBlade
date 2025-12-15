// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCombatComponent.h"
#include "PlayerAnim.generated.h"

// 클래스 전방 선언
class UPlayerCombatComponent;
/**
 *
 */
UCLASS()
class NEOBLADE_API UPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()

public:

	// 매 프레임 갱신되는 함수
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;



	// ========================
	//   플레이어 기본 이동
	// ========================

	// 플레이어 이동 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	float speed = 0;
	// 플레이어 좌우 이동 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	float direction = 0;

	// 플레이어가 공중에 있는지 확인
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	bool bIsFalling;
	// 플레이어가 더블 점프를 하고 있는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	bool bIsDoubleJumping;
	// 점프를 몇번 했는지 ( 일반 점프 / 더블 점프 구분용 )
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	int32 JumpCount;




	// =======================
	//   플레이어 전투 FSM
	// =======================

	// 전투 FSM 상태 관리/처리 컴포넌트
	UPROPERTY(BlueprintReadOnly)
	UPlayerCombatComponent* Combat;


	// 전투 상태 Enum 클래스
	UPROPERTY(BlueprintReadOnly)
	ECombatState AnimCombatState;


	// 공격 타입 Enum (애님 그래프에서 사용)
	UPROPERTY(BlueprintReadOnly)
	EAttackType AnimAttackType = EAttackType::None;

	// 무기 상태 Enum 클래스
	UPROPERTY(BlueprintReadOnly)
	EWeaponState AnimWeaponState;




	// 약공격 애니메이션 몽타주 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* LightAttackMontage;
	// 강공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* HeavyAttackMontage;

	// 공중 약공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* AirLightAttackMontage;
	// 공중 강공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* AirHeavyAttackMontage;

	// 대쉬 공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* DashLightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* DashHeavyAttackMontage;

	// 띄우기 공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* LaunchAttackMontage;
	// 회피 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* DodgeMontage;
	// 피격 당했을 때 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* HitReactMontage;

	// 플레이어 죽음 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Die |Montage")
	UAnimMontage* DeathMontage;


	////  공중 피니시용 몽타주
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	//UAnimMontage* AirFinishMontage;   
// 패링 준비 모션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* ParryStartMontage;

	// 패링 성공 모션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* ParrySuccessMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montage")
	UAnimMontage* GuardHitMontage;

	// ────────────────────────────────────────
	//  몽타주 실행 함수
	// ────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayAnimMontageSafe(UAnimMontage* Montage, FName SectionName = NAME_None);

	/*UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAnimMontageSafe(UAnimMontage* Montage, float BlendOutTime = 0.2f);*/

	// 회피 
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayDodgeMontage();

	// 피격 당했을 때
	UFUNCTION(BlueprintCallable)
	void OnHitReaction();

	// 가드 중에 피격 당했을 시 
	UFUNCTION(BlueprintCallable)
	void PlayGuardHitMontage();

	// 죽음
	UFUNCTION(BlueprintCallable)
	void PlayDieMontage();




	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	bool bIsGuarding = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	bool bIsParrying = false; // 패링 중일때 FSM 구분용





};
