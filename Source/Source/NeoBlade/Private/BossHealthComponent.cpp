// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHealthComponent.h"
#include "Boss1.h"
#include "TimerManager.h"

#include "BossHitReactionComponent.h"


// Sets default values for this component's properties
UBossHealthComponent::UBossHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBossHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// 시작 체력 = 최대 체력
	MaxHP = 1400.f;
	CurrentHP = MaxHP;
}


// Called every frame
void UBossHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBossHealthComponent::ApplyDamage(float Damage)
{
	if (Damage <= 0.f) return;

	// PhaseChange 중이면 데미지 무시
	if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
	{
		if (Boss->BossState == EBossState::PhaseChange)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Damage] Ignored because PhaseChange"));
			return; // <<< 데미지 무시
		}
	}

	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("Boss Health: %f"), CurrentHP);

	// 죽었을 경우
	if (CurrentHP <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Dead"));

		HandleDeath();

		return;
	}

	// 피격 리액션 재생
	if (UBossHitReactionComponent* HitReactComp = GetOwner()->FindComponentByClass<UBossHitReactionComponent>())
	{
		HitReactComp->PlayHitReact();
	}
	// 보스 주인 가져오기
	if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
	{
		if (Boss->CurrentPhase == 1 && CurrentHP <= 1350.0f)
		{
			Boss->CurrentPhase = 2;

			// 1) 상태 변경 (안전하게 SetBossState 사용)
			Boss->SetBossState(EBossState::PhaseChange);

			// 2) Hit 타이머(있다면) 취소: HitReaction 컴포넌트가 있으면 ClearTimer 하자
			if (UBossHitReactionComponent* HitComp = Boss->FindComponentByClass<UBossHitReactionComponent>())
			{
				if (HitComp->HitTimer.IsValid())
					Boss->GetWorldTimerManager().ClearTimer(HitComp->HitTimer);
			}

			// 3) 현재 재생 중인 몽타주 모두 중지
			if (UAnimInstance* AI = Boss->GetMesh()->GetAnimInstance())
				AI->StopAllMontages(0.1f);

			// 4) Phase 종료는 전용 콜백으로 (StopAttack 사용 금지)
			GetWorld()->GetTimerManager().SetTimer(
				PhaseChangeTimer,
				Boss,
				&ABoss1::StopPhaseChange,
				3.0f,
				false
			);

			UE_LOG(LogTemp, Warning, TEXT("[Health] PhaseChange triggered at HP=%f"), CurrentHP);
		}
	}
}

void UBossHealthComponent::HandleDeath()
{
	// 보스 주인 가져오기
	if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
	{
		// 보스 상태를 Hit으로 변경
		Boss->SetBossState(EBossState::Dead);
		// AI 로직 완전 차단
		Boss->SetActorTickEnabled(false);


		// Optional: Hit 애니 재생이 ABP에서 State Machine으로 연결되어 있다면, 몽타주 따로 안해도 됨
	}
}

