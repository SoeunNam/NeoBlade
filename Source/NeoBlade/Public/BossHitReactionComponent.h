// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitReactionComponent.h"
#include "BossHitReactionComponent.generated.h"

class UAnimMontage;
class UBossDamageComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NEOBLADE_API UBossHitReactionComponent : public UHitReactionComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossHitReactionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		private:

    // 피격 애니메이션 몬타주
    UPROPERTY(EditAnywhere, Category="Hit Reaction")
    UAnimMontage* HitMontage;

    // 보스의 스켈레탈메시/애님인스턴스 가져오기용
    UPROPERTY()
    USkeletalMeshComponent* MeshComp;

public:
    // 외부에서 호출: "피격 리액션 실행"
    UFUNCTION(BlueprintCallable)
    void PlayHitReact();

    FTimerHandle HitTimer;
};
