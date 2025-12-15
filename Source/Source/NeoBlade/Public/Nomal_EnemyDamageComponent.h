// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageComponent.h"
#include "Nomal_EnemyDamageComponent.generated.h"

class UNomal_EnemyHealthComponent;
class UNomal_EnemyHitReactionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UNomal_EnemyDamageComponent : public UDamageComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNomal_EnemyDamageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	// 부모 UDamageComponent의 가상 함수 override
	virtual void DealDamage(float Damage, AActor* Causer) override;

	virtual void AttackDealDamage(float Damage, AActor* Causer, EAttackType AttackType) override;

	UNomal_EnemyHealthComponent* HealthComp = nullptr;
	UNomal_EnemyHitReactionComponent* HitReactionComp = nullptr;
};
