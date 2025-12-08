// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.h"     // 부모 헤더 포함
#include "PlayerHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHPChanged, float, NewHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UPlayerHealthComponent : public UHealthComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UPlayerHealthComponent();

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
    FOnHPChanged OnHPChanged;

    UPROPERTY(BlueprintAssignable)
    FOnPlayerDeath OnPlayerDeath;


};

