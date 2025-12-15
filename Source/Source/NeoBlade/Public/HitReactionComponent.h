#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackType.h"
#include "HitReactionComponent.generated.h"

// 피격 반응 종류 타입
UENUM(BlueprintType)
enum class EHitReactionType : uint8
{
    GroundLight     UMETA(DisplayName = "Ground Light Hit"),
    GroundHeavy     UMETA(DisplayName = "Ground Heavy Hit"),

    AirborneLaunch  UMETA(DisplayName = "Launch (Airborne)"),
    AirHit          UMETA(DisplayName = "Air Hit (공중 피격)"),

    KnockDown       UMETA(DisplayName = "Knock Down"),
    ParryStun       UMETA(DisplayName = "ParryStun")
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NEOBLADE_API UHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHitReactionComponent();

protected:
    virtual void BeginPlay() override;

public:

    //★ 반드시 이 함수가 있어야 override 가능 ★
    UFUNCTION(BlueprintCallable)
    virtual void PlayHitReaction();


    virtual void PlayHitReaction(EAttackType AttackType);


    virtual void  EnterParryStun();


};
