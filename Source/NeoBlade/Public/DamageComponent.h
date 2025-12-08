// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackType.h"
#include "HitReactionComponent.h"
#include "DamageComponent.generated.h"

class UPlayerHealthComponent;
class UHitReactionComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDamageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere)
	float BaseDamage = 10.f;

	virtual void DealDamage(float Damage, AActor* Causer);

	// 공격 타입을 받아 적에게 전달 
	virtual void AttackDealDamage(float Damage, AActor* Causer, EAttackType AttackType);
	// 어떤 타입의 공격인지 변환하여 Hit리터랙션 행동반응 클래스에게 전달
	EHitReactionType ConvertAttackToReaction(EAttackType Type);

};
