// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuardShieldComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UGuardShieldComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UGuardShieldComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    //virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:


    // 내구도
    int32 MaxShield = 3;
    int32 CurrentShield = 3;

    // 쿨타임
    float ShieldCooldown = 5.f;
    bool bShieldOnCooldown = false;

    // 타이머
    FTimerHandle ShieldCooldownTimer;


    // Shield Mesh (반투명 구체)
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* ShieldMesh;

public:

    bool CanActivateShield() const;
    void ActivateShield();
    void DeactivateShield();

    // 공격 맞았을 때 호출
    bool OnGuardHit(AActor* Attacker);

private:
    void StartCooldown();
    void ResetShield();

};
