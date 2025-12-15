//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NiagaraComponent.h"
#include <NiagaraFunctionLibrary.h>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DroneEnemyFSM.generated.h"


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Move,
	Attack,
	Damage,
	Die,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UDroneEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDroneEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	// 상태변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FSM)
	EEnemyState mState = EEnemyState::Idle;

	void IdleState();
	void MoveState(float DeltaTime);
	void AttackState();
	void DamageState();
	void DieState();

	UPROPERTY(EditDefaultsOnly, Category = FSM)
	float idleDelayTime = 2;
	float currentTime = 0;

	// 타겟
	UPROPERTY(VisibleAnywhere, Category = FSM)
	class APlayerCharacter* target;
	// 소유액터
	UPROPERTY()
	class ADroneEnemy* me;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawn)
	FString id;

	//이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawn)
	float moveSpeed = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawn)
	float minSpeed = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawn)
	float maxSpeed = 1.f;
	/*UFUNCTION()
	void SetMoveSpeed(float value);*/

	// 공격범위
	UPROPERTY(EditAnywhere, Category = FSM)
	float attackRange = 150.0f;
	// 공격대기시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float attackDelayTime = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSM)
	FVector destination;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSM)
	FVector dir;



	// 피격 알림 이벤트 함수
	void OnDamageProcess();

	// 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	int32 maxHp = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	int32 hp = maxHp;
	// 피격대기시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	int32 maxMp = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	int32 mp = maxHp;
	// 피격대기시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float damageDelayTime = 2.0f;
	UPROPERTY(EditAnywhere, Category = FSM)
	float mpDelayTime = 2.0f;
	// 아래로 사라지는 속도
	UPROPERTY(EditAnywhere, Category = FSM)
	float dieSpeed = 50.0f;

	// 사용중인 애니메이션 블루프린트
	UPROPERTY()
	class UEnemyAnim* anim;

	UPROPERTY(EditAnywhere, Category = Animation)
	class UAnimMontage* damageAnim;

	// Enemy 를 소유하고 있는 AIController
	//UPROPERTY()
	//class AAIController* ai;

	// 길찾기 수행시 랜덤위치
	FVector randomPos;
	// 랜덤위치 가져오기
	//bool GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest);

	UPROPERTY(EditAnywhere, Category = Effect)
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	UNiagaraSystem* NiagaraExplosionEffect;
		
};
