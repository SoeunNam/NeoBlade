// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitReactionComponent.h"
#include "Nomal_EnemyHitReactionComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UNomal_EnemyHitReactionComponent : public UHitReactionComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNomal_EnemyHitReactionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void PlayHitReaction() override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void PlayHitReaction(EAttackType AttackType) override;

	// 에어본에 맞았을 때 Enemy의 피격 반응 : 공중에 떠올랐다가 착지
	void LaunchIntoAir(AActor* Causer);

	// 공중공격 피격반응 : 공중에서 잠깐 유지 
	void SustainAirborneHit();

	// 공중공격 마지막 콤보를 받았을 때 피격 반응 : 바닥으로 내리꽂힘
	void AirFinishHit();

private:

	// Slam 후 바닥 충돌 감지를 위한 상태 플래그
	bool bIsInAirSlam = false;

};
