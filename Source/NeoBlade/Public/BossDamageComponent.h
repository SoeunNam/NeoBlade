// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageComponent.h"
#include "BossDamageComponent.generated.h"

// 보스 데미지 처리 담당 컴포넌트
// - 외부에서 들어온 데미지를 받아서 체력 컴포넌트에 전달해주는 역할만 함
// - 데미지 공식(방어력, 배율 등)을 여기서 처리해도 됨

class UBossHitReactionComponent;
class UBossHealthComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UBossDamageComponent : public UDamageComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossDamageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 같은 액터에 붙어있는 체력 컴포넌트를 포인터로 저장
	UPROPERTY()
	UBossHealthComponent* HealthComp = nullptr;
	UBossHitReactionComponent* HitReactionComp = nullptr;

public:
	// 외부에서 데미지를 받을 때 사용하는 함수
	UFUNCTION(BlueprintCallable)
	void ApplyBossDamage(float Damage);
};
