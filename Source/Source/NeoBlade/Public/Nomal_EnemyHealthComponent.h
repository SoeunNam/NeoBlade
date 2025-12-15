// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "Nomal_EnemyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNomal_EnemyHPChanged, float, NewHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNomal_EnemyDeath);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UNomal_EnemyHealthComponent : public UHealthComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNomal_EnemyHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	virtual void ApplyDamage(float Damage) override;
	virtual void HandleDeath() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnNomal_EnemyHPChanged OnNomal_EnemyHPChanged;

	UPROPERTY(BlueprintAssignable)
	FOnNomal_EnemyDeath OnNomal_EnemyDeath;

};
