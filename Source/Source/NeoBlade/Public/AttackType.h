#pragma once

#include "CoreMinimal.h"
#include "AttackType.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Light       UMETA(DisplayName = "LightAttack"),
    Heavy       UMETA(DisplayName = "HeavyAttack"),
    AirLight    UMETA(DisplayName = "AirLightAttack"),
    AirHeavy    UMETA(DisplayName = "AirHeavyAttack"),
    AirFinish   UMETA(DisplayName = "AirFinishHit"),
    Launch      UMETA(DisplayName = "Launch"),
    DashLight   UMETA(DisplayName = "DashLightAttack"),
    DashHeavy   UMETA(DisplayName = "DashHeavyAttack"),
    Parry      UMETA(DisplayName = "Parry")
};
