// Fill out your copyright notice in the Description page of Project Settings.



#include "MeleeEnemy.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EnemyAnim.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/BoxComponent.h"
#include "SwordTrailActor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Nomal_EnemyHealthComponent.h"
#include "Nomal_EnemyDamageComponent.h"
#include "Nomal_EnemyHitReactionComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Components/CapsuleComponent.h"
#include"RoomManager.h"
#include"UCombatManagerComponent.h"
#include"UCombatSlotComponent.h"
#include "GameFramework/GameStateBase.h"



AMeleeEnemy::AMeleeEnemy()
{



	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 체력 관리 클래스 적에게 부착
	HealthComp = CreateDefaultSubobject<UNomal_EnemyHealthComponent>(TEXT("HealthComponent"));
	// 데미지 관리 클래스 적에게 부착
	DamageComp = CreateDefaultSubobject<UNomal_EnemyDamageComponent>(TEXT("DamageComponent"));
	// 피격 반응 관리 클래스 적에게 부착 
	HitReactionComp = CreateDefaultSubobject<UNomal_EnemyHitReactionComponent>(TEXT("HitReactionComponent"));

	CombatSlot = CreateDefaultSubobject<UUCombatSlotComponent>("CombatSlot");
	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsObject("TargetActor", TargetActor);
}


// Called when the game starts or when spawned
void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();
	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (SwordClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		SwordActor = GetWorld()->SpawnActor<ASwordTrailActor>(SwordClass, Params);

		if (SwordActor)
		{
			SwordActor->SetActorScale3D(FVector(0.15f, 0.15f, 0.15f));  // 원하는 비율
			SwordActor->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FName("Sword")  // 캐릭터 손에 있는 소켓 이름
			);
		}


	}
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsObject("TargetActor", TargetActor);

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		UUCombatManagerComponent* Manager =
			GS->FindComponentByClass<UUCombatManagerComponent>();

		if (Manager && CombatSlot)
		{
			CombatSlot->Manager = Manager;
			CombatSlot->RequestAttack();   // ✅ 무조건 여기서 요청만!
		}
	}

}
void AMeleeEnemy::PlaySlashEffect(int32 _value)
{
	if (!SlashEffect || !SwordActor) return;

	// 소켓 위치/회전
	if (!SlashEffect || !SwordActor) return;


	// 1) 소켓 기준 위치/회전 가져오기
	FVector Loc = GetActorLocation() + GetActorForwardVector() * 100.0f;
	FRotator Rot = GetActorRotation();

	// 2) 상대 위치 오프셋 적용
	if (_value == 1)
	{
		Rot.Pitch += -180;
	}
	else if (_value == 2)
	{
		// 3) 상대 회전 오프셋 적용
		Rot.Pitch += -180;
		Rot.Yaw += 180;
	}

	// 4) 이펙트 스폰
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SlashEffect,
		Loc,
		Rot,
		FVector(1.0f)
	);
}
void AMeleeEnemy::SetSpeed(float _Speed)
{
	if (UEnemyAnim* Anim = Cast<UEnemyAnim>(GetMesh()->GetAnimInstance()))
	{
		Anim->Speed = _Speed;     // ← 애니메이션 블루프린트 변수로 값 전달
	}
}

// Called every frame
void AMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!TargetActor || bIsDead) return;

	const float DistSq = FVector::DistSquared(TargetActor->GetActorLocation(), GetActorLocation());



	const float AttackSq = AttackRange * AttackRange;
	const float CircleSq = CircleRange * CircleRange;


	// 새로운 상태 계산
	bool bNewAttack = (DistSq < AttackSq);
	const bool bInCircleRange = (DistSq < CircleSq);



	if (GEngine)
	{
		FString DetectStr = FString::Printf(TEXT("Detect: %s | Attack: %s"),
			bIsInDetectRange ? TEXT("TRUE") : TEXT("FALSE"),
			bIsInAttackRange ? TEXT("TRUE") : TEXT("FALSE"));

		GEngine->AddOnScreenDebugMessage(
			2,          // Key (1이면 계속 같은 메시지를 갱신)
			0.0f,       // Duration (0 = 프레임마다 갱신)
			FColor::Yellow,
			DetectStr
		);
	}
	/*DebugRanges();*/
	AAIController* AI = Cast<AAIController>(GetController());
	UBlackboardComponent* BB = nullptr;
	if (AI)
	{
		BB = AI->GetBlackboardComponent();
	}
	// 공격 범위 판정은 무조건 갱신해야 함
	bIsInAttackRange = bNewAttack;
	if (GEngine)
	{
		FString SlotStr = FString::Printf(TEXT("CanAttack: %s"),
			bNewAttack ? TEXT("TRUE") : TEXT("FALSE"));

		GEngine->AddOnScreenDebugMessage(
			3, 0.f, FColor::Green, SlotStr
		);
	}
	// 공격권이 있는 경우만 실제 공격 상태로 들어감
	if (bNewAttack == true && CombatSlot && CombatSlot->bCanAttack)
	{
		bIsReady = false;  // 원형 끊기
		UpdateAttackState(true);   // 공격자만 공격 상태
	}


	// 공격권이 없고, 일정 거리 안이면 → 돌기 모드
	if (bInCircleRange && !(CombatSlot && CombatSlot->bCanAttack))
	{
		if (BB)
			BB->SetValueAsBool("bIsReady", true);

	
	}
	else
	{
		if (BB)
		BB->SetValueAsBool("bIsReady", false);

		bIsReady = false;
	}


	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (bIsAttacking && Anim && !Anim->Montage_IsPlaying(AttackMontage))
	{
		// -> 강제 AttackEnd 처리
		ForceAttackEnd();
	}
	if (bNewAttack == false)
	{
		CheckPlayerInNavMesh();

	}
	if (bIsReady)   // 원형 회전 상태
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		CircleAroundPlayer(DeltaTime);
	}
	else
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		ResetCircleState();
	}
}
void AMeleeEnemy::CircleAroundPlayer(float DeltaTime)
{
	if (!TargetActor) return;

	FVector PlayerPos = TargetActor->GetActorLocation();
	FVector ToPlayer = PlayerPos - GetActorLocation();
	float CurrentDist = ToPlayer.Size();

	FVector RightDir = FVector::CrossProduct(ToPlayer.GetSafeNormal(), FVector::UpVector);

	FVector MoveDir = GetVelocity().GetSafeNormal();
	CurrentOrbitDirection = FVector::DotProduct(MoveDir, RightDir);
	// -----------------------------
	// ⭐ 0) 회전 보정 (가장 중요한 부분)
	// -----------------------------
	FRotator LookRot = (PlayerPos - GetActorLocation()).Rotation();
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), LookRot, DeltaTime, 3.0f);
	SetActorRotation(NewRot);

	// ---- 1) 랜덤 멈춤 ----
	if (PauseTimer > 0.f)
	{
		PauseTimer -= DeltaTime;
		return;
	}

	// ---- 2) 속도 목표 랜덤 설정 ----
	if (FMath::IsNearlyZero(TargetSpeed))
	{
		TargetSpeed = FMath::RandRange(80.f, 160.f) * DirectionSign;

		if (FMath::RandRange(0.f, 1.f) < 0.1f)
		{
			DirectionSign *= -1;
		}
	}

	// ---- 가속/감속 ----
	CircleSpeed = FMath::FInterpTo(CircleSpeed, TargetSpeed, DeltaTime, 0.8f);

	// ---- 3) 랜덤 멈춤 ----
	if (FMath::RandRange(0.f, 1.f) < 0.002f)
	{
		PauseTimer = FMath::RandRange(0.2f, 0.8f);
		TargetSpeed = 0.f;
		return;
	}

	// ---- 4) 이동 ----
	AddMovementInput(RightDir, CircleSpeed * 0.01f);

}
void AMeleeEnemy::ResetCircleState()
{
	// 원형 이동 관련 모든 값 초기화
	CircleSpeed = 0.f;
	TargetSpeed = 0.f;
	PauseTimer = 0.f;

	// 방향도 다시 랜덤으로 설정
	DirectionSign = (FMath::RandBool() ? 1.f : -1.f);
}
void AMeleeEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	UpdateMiddleState(false);
}
void AMeleeEnemy::PlayHitMontage()
{

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);  // 0.1초 블렌드 아웃
	}
	if (HitMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitMontage);
	}
	bIsMiddleState = true;   // 너가 말한 "중도" 상태 넣어도 됨
	UpdateMiddleState(bIsMiddleState);

	// Hit이 끝나면 다시 움직일 수 있도록 타이머
	float HitDuration = HitMontage ? HitMontage->GetPlayLength() : 0.5f;
	FTimerHandle HitTimer;
	GetWorld()->GetTimerManager().SetTimer(HitTimer, [&]()
		{

			bIsMiddleState = false;
			UpdateMiddleState(bIsMiddleState);
			UpdateAttackState(false);
		}, HitDuration + 1.0f, false);
}

void AMeleeEnemy::PlayAirAttackedHitMontage()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);  // 0.1초 블렌드 아웃
	}
	if (AirAttackedMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AirAttackedMontage);
	}
	UpdateMiddleState(true);


}

void AMeleeEnemy::PlayAirBorneHitMontage()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);  // 0.1초 블렌드 아웃
	}
	if (AirBorneMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AirBorneMontage);
	}
	UpdateMiddleState(true);
}

void AMeleeEnemy::PlayFinalAttackHitMontage()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;

	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);
	}

	if (FinalAttackMontage)
	{
		// 1) 몽타주 재생 + 실제 재생 길이 획득
		float Duration = Anim->Montage_Play(FinalAttackMontage);

		// 2) 중간 상태 업데이트
		UpdateMiddleState(true);
		FTimerHandle TimerHandle_FinalAttack;
		// 3) 몽타주가 끝난 후 다음 행동 실행 (타이머)
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_FinalAttack,
			this,
			&AMeleeEnemy::OnFinalAttackFinished,
			Duration,
			false
		);
	}
}

void AMeleeEnemy::PlayWakeUpMontage()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);  // 0.1초 블렌드 아웃
	}
	if (WakeUpMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(WakeUpMontage);
	}
	UpdateMiddleState(true);
}

void AMeleeEnemy::Die()
{
	bIsDead = true;
	// ✅ 메시도 플레이어랑은 안 부딪히게 설정 (시체는 보이기만 함)

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ✅ 메시는 원래대로 NoCollision 유지
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	if (DeathMontage)
	{

		GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
	}
	if (DeathMontage && GEngine)
	{
		float PlayResult = GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);

		bool bPlaying = (PlayResult > 0.f);

		FString MontageStr = FString::Printf(
			TEXT("AttackMontage Playing: %s"),
			bPlaying ? TEXT("TRUE") : TEXT("FALSE")
		);

		GEngine->AddOnScreenDebugMessage(
			10,          // Key: 같은 키면 매 프레임 갱신
			0.0f,        // Duration: 0이면 한 프레임, 매 Tick 호출로 계속 업데이트
			FColor::Cyan,
			MontageStr
		);
	}
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		auto BB = AI->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool("IsDead", true);
		}

	}
	// ✅ 방 매니저 찾기
	TArray<AActor*> Rooms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomManager::StaticClass(), Rooms);

	if (Rooms.Num() > 0)
	{
		ARoomManager* Room = Cast<ARoomManager>(Rooms[0]);
		if (Room)
		{
			Room->OnEnemyDead();
		}
	}
	if (CombatSlot && CombatSlot->Manager)
	{
		CombatSlot->Manager->OnEnemyDeath(CombatSlot);
	}
}

void AMeleeEnemy::DebugRanges()
{
	FVector MyLoc = GetActorLocation();
	FVector TargetLoc = TargetActor->GetActorLocation();



	// 공격 범위
	DrawDebugSphere(GetWorld(), MyLoc, AttackRange, 32, FColor::Red, false, 0.0f, 0, 2);

	// 적 ↔ 플레이어 선
	DrawDebugLine(GetWorld(), MyLoc, TargetLoc, FColor::Yellow, false, 0.0f, 0, 3);
}

void AMeleeEnemy::OnAttackEnd()
{


	// MoveTo 재시작 위해 컨트롤러 동기화

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		auto BB = AI->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool("bIsAttack", false);
		}

	}
}

void AMeleeEnemy::ForceAttackEnd()
{
	// 공격 종료 상태로 변경
	bIsInAttackRange = false;
	bIsAttacking = false;
	// 이동/회전 복구

	// BT 갱신하게 Blackboard false로 바꾸기
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		auto BB = AI->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool("bIsAttack", false);
		}


	}
}

void AMeleeEnemy::OnFinalAttackFinished()
{
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (!Anim) return;
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		Anim->Montage_Stop(0.1f, AttackMontage);  // 0.1초 블렌드 아웃
	}
	if (WakeUpMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(WakeUpMontage);
	}
	UpdateMiddleState(true);
}

void AMeleeEnemy::CheckPlayerInNavMesh()
{
	if (!TargetActor) return;

	FVector PlayerPos = TargetActor->GetActorLocation();

	// 네비게이션 시스템 가져오기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	// NavMesh 데이터 가져오기
	const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!NavData) return;

	// NavMesh 안에 있는지 검사
	FNavLocation Result;
	bool bInNavMesh = NavSys->ProjectPointToNavigation(
		PlayerPos,
		Result,
		FVector(50, 50, 200)
	);
	/*if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (CombatSlot && CombatSlot->bCanAttack == false)
		{
			auto BB = AI->GetBlackboardComponent();
			if (BB)
			{
				BB->SetValueAsBool("bIsReady", true);
			}
		}

	}*/
	// 결과 적용
	if (bInNavMesh)
	{
		UpdateDetectState(true);   // 블랙보드 true
	}
	else
	{
		UpdateDetectState(false);  // 블랙보드 false
	}
}

void AMeleeEnemy::AttackHitCheck()
{

}



void AMeleeEnemy::Attack()
{
	if (bIsDead) return;
	if (AttackMontage == nullptr) return;
	bIsAttacking = true;
	if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage))
	{



		// 몽타주 재생
		GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);
	}
}

void AMeleeEnemy::UpdateDetectState(bool _value)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	if (UEnemyAnim* Anim = Cast<UEnemyAnim>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsAttackingState = _value;     // ← 애니메이션 블루프린트 변수로 값 전달
	}

	BB->SetValueAsBool("bIsDetected", _value);
}

void AMeleeEnemy::UpdateAttackState(bool _value)
{

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsBool("bIsAttack", _value);

}

void AMeleeEnemy::UpdateMiddleState(bool _value)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();
	BB->SetValueAsBool("bIsMiddelState", _value);

}

void AMeleeEnemy::StartSwordTrail()
{
	if (SwordActor)
		SwordActor->EnableTrail();
}

void AMeleeEnemy::EndSwordTrail()
{
	if (SwordActor)
		SwordActor->DisableTrail();
}

void AMeleeEnemy::LineTrace()
{
	SwordActor->SwordLineTrace();
}

void AMeleeEnemy::OnDetectBegin(UPrimitiveComponent* OverComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit)
{

}

void AMeleeEnemy::OnDetectEnd(UPrimitiveComponent* OverComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{

}
