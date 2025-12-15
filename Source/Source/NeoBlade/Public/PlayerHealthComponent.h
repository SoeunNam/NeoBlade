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
    // 데미지 적용 시 HP 변화를 BP/UI에 알리기 위한 이벤트
    UPROPERTY(BlueprintAssignable, Category = "HP")
    FOnHPChanged OnHPChanged;

    // 플레이어 사망 이벤트 (PlayerCharacter에서 바인딩)
    UPROPERTY(BlueprintAssignable, Category = "HP")
    FOnPlayerDeath OnPlayerDeath;


};

