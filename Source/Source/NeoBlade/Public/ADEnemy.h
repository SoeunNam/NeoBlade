// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ADEnemy.generated.h"

class AADProjectile;   // Projectile 클래스 포워드 선언
class USceneComponent;
class UNomal_EnemyHealthComponent; // 플레이어 체력 관리
class UNomal_EnemyDamageComponent; // 플레이어 데미지 관리 
class UNomal_EnemyHitReactionComponent; //  플레이어  타격 반응 관리


UCLASS()
class NEOBLADE_API AADEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AADEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	USceneComponent* MuzzlePoint;
	// Attack Montage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detect")
	class UBoxComponent* DetectBox;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsLasing;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bisAirborne = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool isEnemyDead = false;

	// Target Player
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AActor* TargetActor;

	// Patrol & Detection
	UPROPERTY(EditAnywhere, Category = "AI")
	float DetectionRange = 600.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackRange = 1300.f;


	// Attack Montage
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DeathMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* FinalAttackMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AirBorneMontage;
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AirAttackedMontage;


	class UNiagaraComponent* LaserComp;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* LaserEffect; // 만들어둔 Beam Niagara System


public:
	// === Functions ===
	void Attack();
	void UpdateDetectState(bool _value);
	void UpdateAttackState(bool _value);
	void UpdateMiddleState(bool _value);
	void PlayHitMontage();
	void A_PlayAirAttackedHitMontage();
	void A_PlayAirBorneHitMontage();
	void A_PlayFinalAttackHitMontage();
	void Die();
	void UpdateAttackRange();
	void DebugRanges();
	void OnAttackEnd();
	void ForceAttackEnd();
	void CheckPlayerInNavMesh();
	void SetSpeed(float _Speed);
	void Landed(const FHitResult& Hit);
	// === Functions ===
	void Attack_ReloadStep();
	void Attack_ShootStep();
	void Attack_StandUpStep();
	void Attack_End();
	void DrawAimLaser();
	void UpdateLaserEffect();


	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void Notify_Shoot();



	UFUNCTION()
	void ShootProjectile();
	UFUNCTION()
	bool CheckAttackRange();



	UFUNCTION()
	void OnDetectBoxBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnDetectBoxEnd(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);




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
