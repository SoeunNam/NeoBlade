// PlayerDamageComponent.h

#pragma once

#include "CoreMinimal.h"
#include "DamageComponent.h"
#include "PlayerDamageComponent.generated.h"

class UPlayerHealthComponent;
class UPlayerHitReactionComponent;
class UPlayerCombatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UPlayerDamageComponent : public UDamageComponent
{
    GENERATED_BODY()

public:
    UPlayerDamageComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 부모 UDamageComponent의 가상 함수 override
    virtual void DealDamage(float Damage, AActor* Causer) override;

private:
    UPlayerHealthComponent* HealthComp = nullptr;
    UPlayerHitReactionComponent* HitReactionComp = nullptr;
    UPlayerCombatComponent* CombatComp = nullptr;
};
