#pragma once

#include "CoreMinimal.h"
#include "HitReactionComponent.h"
#include "PlayerHitReactionComponent.generated.h"

class UPlayerAnim;

UCLASS()
class NEOBLADE_API UPlayerHitReactionComponent : public UHitReactionComponent
{
    GENERATED_BODY()

public:
    UPlayerHitReactionComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ★ 플레이어 피격 반응 (부모 override)
    virtual void PlayHitReaction() override;

private:
    UPlayerAnim* PlayerAnim;   // AnimInstance 캐싱
};
