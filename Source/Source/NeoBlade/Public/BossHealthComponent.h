// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Boss1.h"
#include "BossHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDeath);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UBossHealthComponent : public UHealthComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 최대 체력
    float MaxHP = 2700.f;

    // 현재 체력
    float CurrentHP;

public:
    // 체력 감소 처리 함수
    UFUNCTION(BlueprintCallable)
    void ApplyDamage(float Damage) override;

    // 죽었을 때 Blueprint에서도 사용 가능한 델리게이트
    UPROPERTY(BlueprintAssignable)
    FOnBossDeath OnBossDeath;

    // 현재 체력 가져오기
    UFUNCTION(BlueprintCallable)
    float GetHealth() const { return CurrentHP; }
	
    UFUNCTION()
    void HandleDeath() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    EBossState BossState = EBossState::Idle;

    // 패턴 종료용 타이머
    FTimerHandle PhaseChangeTimer;
 };
