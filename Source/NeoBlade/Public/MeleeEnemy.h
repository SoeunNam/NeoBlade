// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MeleeEnemy.generated.h"

class UBoxComponent;
class UNomal_EnemyHealthComponent; // 플레이어 체력 관리
class UNomal_EnemyDamageComponent; // 플레이어 데미지 관리 
class UNomal_EnemyHitReactionComponent; //  플레이어  타격 반응 관리

UCLASS()
class NEOBLADE_API AMeleeEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
public:
	// Sets default values for this character's properties
	AMeleeEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// === Enemy Stats ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;
	// === AI Variables ===
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bHasTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsDead = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsHit = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsattack = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsInAttackRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsInDetectRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsMiddleState = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsAttacking;

	// Target Player
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AActor* TargetActor;

	// Patrol & Detection
	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRange = 600.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRange = 100.f;


	// Attack Montage
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ASwordTrailActor> SwordClass;
	UPROPERTY(EditAnywhere, Category = "FX")
	class UNiagaraSystem* SlashEffect;

	UPROPERTY()
	ASwordTrailActor* SwordActor;
public:
	// === Functions ===
	void Attack();
	void UpdateDetectState(bool _value);
	void UpdateAttackState(bool _value);
	void UpdateMiddleState(bool _value);
	void PlayHitMontage();
	void Die();
	void DebugRanges();
	void OnAttackEnd();
	void ForceAttackEnd();
	void CheckPlayerInNavMesh();
	void PlaySlashEffect(int32 _value);
	void SetSpeed(float _Speed);


	// AnimNotify 에서 부를 함수
	UFUNCTION()
	void AttackHitCheck();
	UFUNCTION()
	void StartSwordTrail();

	UFUNCTION()
	void EndSwordTrail();
	// 데미지 받기

	void LineTrace();





	UPROPERTY(EditAnywhere, Category = "Rotator")
	float YawOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float PitchOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float RollOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float ForwardOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float UpOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float RightOffset = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float YawOffset2 = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float PitchOffset2 = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float RollOffset2 = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float ForwardOffset2 = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float UpOffset2 = 0;
	UPROPERTY(EditAnywhere, Category = "Rotator")
	float RightOffset2 = 0;

	UFUNCTION()
	void OnDetectBegin(UPrimitiveComponent* OverComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnDetectEnd(UPrimitiveComponent* OverComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 BodyIndex);
	// =====================================
	//  적 체력 & 데미지 & 피격 반응 관리 컴포넌트
	// =====================================

	// 플레이어 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNomal_EnemyHealthComponent* HealthComp;
	// 플레이어 공통 데미지 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNomal_EnemyDamageComponent* DamageComp;
	// 플레이어  피격 반응
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNomal_EnemyHitReactionComponent* HitReactionComp;
};
