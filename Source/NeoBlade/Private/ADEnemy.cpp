// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "ADEnemy.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Projectile.h"                      // Projectile 스폰
#include "Animation/AnimInstance.h"            // Montage_Play
#include "TimerManager.h"                      // 타이머
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "EnemyAnim.h"
#include "DrawDebugHelpers.h"
#include "Nomal_EnemyHealthComponent.h"
#include "Nomal_EnemyDamageComponent.h"
#include "Nomal_EnemyHitReactionComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AADEnemy::AADEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 체력 관리 클래스 적에게 부착
	HealthComp = CreateDefaultSubobject<UNomal_EnemyHealthComponent>(TEXT("HealthComponent"));
	// 데미지 관리 클래스 적에게 부착
	DamageComp = CreateDefaultSubobject<UNomal_EnemyDamageComponent>(TEXT("DamageComponent"));
	// 피격 반응 관리 클래스 적에게 부착 
	HitReactionComp = CreateDefaultSubobject<UNomal_EnemyHitReactionComponent>(TEXT("HitReactionComponent"));

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));

	// 어디에 붙일지 결정
	// 예: Hand_R 소켓에 붙이기
	MuzzlePoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AADEnemy::BeginPlay()
{
	Super::BeginPlay();


	//DetectBox->OnComponentBeginOverlap.AddDynamic(this, &AADEnemy::OnDetectBoxBegin);
	//DetectBox->OnComponentEndOverlap.AddDynamic(this, &AADEnemy::OnDetectBoxEnd);
	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsObject("EnemyActor", TargetActor);
	// 애니메이션 인스턴스 가져오기
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		Anim->OnMontageEnded.AddDynamic(this, &AADEnemy::OnMontageEnded);
	}
	if (LaserEffect)
	{
		LaserComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LaserEffect,
			MuzzlePoint,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true  // 처음엔 꺼둠
		);
		LaserComp->SetVisibility(false);
	}
}

// Called every frame
void AADEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (bIsAttacking && Anim && !Anim->Montage_IsPlaying(AttackMontage))
	{
		// -> 강제 AttackEnd 처리
		ForceAttackEnd();
	}

	CheckPlayerInNavMesh();
	UpdateAttackRange();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			3,
			4.0f,
			FColor::Red,
			FString::Printf(TEXT("CanSeePlayer = %s"), bIsLasing ? TEXT("true") : TEXT("false"))
		);
	}
	if (bIsLasing == true)
	{

		DrawAimLaser();
	}
	/*if (bIsLasing && LaserComp)
	{
		UpdateLaserEffect();
	}
	else if (LaserComp)
	{
		LaserComp->SetVisibility(false);
	}*/
}
void AADEnemy::SetSpeed(float _Speed)
{
	if (UEnemyAnim* Anim = Cast<UEnemyAnim>(GetMesh()->GetAnimInstance()))
	{
		Anim->Speed = _Speed;     // ← 애니메이션 블루프린트 변수로 값 전달
	}
}
// Called to bind functionality to input
void AADEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void AADEnemy::UpdateLaserEffect()
{
	if (!LaserComp || !LaserEffect || !TargetActor) return;

	FVector Start = MuzzlePoint->GetComponentLocation();
	FVector End = TargetActor->GetActorLocation();
	End.Z += 40.f; // 가슴높이 조절

	LaserComp->SetVisibility(true);

	LaserComp->SetVariableVec3(FName("Enemy"), Start);
	LaserComp->SetVariableVec3(FName("Player"), End);
}
void AADEnemy::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	if (Montage == AttackMontage)
	{
		// 공격 몽타주가 끝났으니 false로 돌려놓기

		UpdateAttackState(false);


		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 2.f, FColor::Yellow, TEXT("Attack Montage Ended!")
			);
		}
	}
}
void AADEnemy::UpdateAttackRange()
{
	if (bIsAttacking) return;
	if (!TargetActor) return;
	const FVector MyLoc = GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation();

	// ---- ① 거리 계산 ----
	const float DistSq = FVector::DistSquared(MyLoc, TargetLoc);
	const float AttackSq = AttackRange * AttackRange;

	// ---- ② 층 구분 ----
	float HeightDiff = FMath::Abs(TargetLoc.Z - MyLoc.Z);
	const float MaxSameFloorHeight = 400.f;
	bool bSameFloor = (HeightDiff <= MaxSameFloorHeight);

	// ---- ③ 시야 확인(LineTrace) ----
	bool bHasLineOfSight = false;

	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(TargetActor); // 필요 시 제거

		FHitResult Hit;

		// 눈 위치 보정
		FVector Start = MyLoc + FVector(0, 0, 60);
		FVector End = TargetLoc + FVector(0, 0, 60);

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Params
		);

		// ---- 🔍 디버그 라인 그리기 ----
		FColor DebugColor;

		if (!bHit)
		{
			// 아무것도 안 맞았다 → 경로가 뚫려있다
			DebugColor = FColor::Green;
			bHasLineOfSight = true;
		}
		else if (Hit.GetActor() == TargetActor)
		{
			// 맞긴 했는데 TargetActor임 → 뚫려있음
			DebugColor = FColor::Green;
			bHasLineOfSight = true;
		}
		else
		{
			// 벽 또는 장애물에 막힘
			DebugColor = FColor::Red;
			bHasLineOfSight = false;
		}


	}

	// ---- ④ 최종 공격 범위 판정 ----
	bool bNewInAttackRange = (DistSq <= AttackSq) && bSameFloor && bHasLineOfSight;

	if (bNewInAttackRange)
	{
		UpdateAttackState(true);
	}
	else
	{
		Attack_End();
	}
}


void AADEnemy::OnDetectBoxBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{

	// Player 만 감지하고 싶으면:
	if (!OtherActor->ActorHasTag("Player")) return;
	bIsattack = true;
	UpdateAttackState(true);
}

void AADEnemy::OnDetectBoxEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{

}


// Called every frame


void AADEnemy::PlayHitMontage()
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
	UpdateMiddleState(true);
	Attack_End();
	// Hit이 끝나면 다시 움직일 수 있도록 타이머
	float HitDuration = HitMontage ? HitMontage->GetPlayLength() : 0.5f;
	FTimerHandle asd;
	GetWorld()->GetTimerManager().SetTimer(asd, [&]()
		{

			bIsMiddleState = false;
			UpdateMiddleState(false);
			/*UpdateAttackState(false);*/
		}, HitDuration + 1.0f, false);
}

void AADEnemy::Die()
{
	bIsDead = true;

	GetCharacterMovement()->DisableMovement();

	if (DeathMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
	}

	// 3초 뒤 소멸
	SetLifeSpan(3.0f);
}
void AADEnemy::OnAttackEnd()
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

void AADEnemy::Notify_Shoot()
{
	ShootProjectile();
}



void AADEnemy::ForceAttackEnd()
{
	// 공격 종료 상태로 변경
	bIsInAttackRange = false;
	bIsAttacking = false;
	// 이동/회전 복구

	// BT 갱신하게 Blackboard false로 바꾸기
	UpdateAttackState(false);
}
void AADEnemy::CheckPlayerInNavMesh()
{
	if (!TargetActor) return;
	if (bIsattack == true) return;
	if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage)) return;
	FVector PlayerPos = TargetActor->GetActorLocation();

	// 네비게이션 시스템 가져오기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	// NavMesh 데이터 가져오기
	const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!NavData) return;

	// NavMesh 안에 있는지 검사
	FNavLocation Result;
	bool bInNavMesh = NavSys->ProjectPointToNavigation(PlayerPos, Result, FVector(50, 50, 200));

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
void AADEnemy::ShootProjectile()
{
	if (!ProjectileClass) return;
	if (!MuzzlePoint) return;
	if (!TargetActor) return;

	FVector SpawnLocation = MuzzlePoint->GetComponentLocation();

	// 플레이어 방향으로 회전
	FVector Dir = (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = Dir.Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Projectile 생성
	GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		Params
	);
}

bool AADEnemy::CheckAttackRange()
{

	if (!TargetActor) return false;

	float Distance = FVector::Distance(GetActorLocation(), TargetActor->GetActorLocation());
	return Distance <= AttackRange;   // 공격거리 안일 때만 true
}


void AADEnemy::Attack()
{
	if (bIsDead) return;
	if (AttackMontage == nullptr) return;

	if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage)) return;

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	bIsLasing = true;
	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsBool("bisAttackEnd", false);
	UpdateDetectState(false);

	PlayAnimMontage(AttackMontage, 1.f, FName("Sit"));  // 섹션: Sit

	// 앉는 모션이 끝나면 다음 단계로
	float SitDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("Sit")));
	FTimerHandle TimerStep1;
	GetWorld()->GetTimerManager().SetTimer(TimerStep1, this, &AADEnemy::Attack_ReloadStep, SitDuration, false);
}

void AADEnemy::Attack_ReloadStep()
{

	// 플레이어가 멀어져 공격 취소되면 바로 종료
	if (!CheckAttackRange())
	{
		Attack_End();
		return;
	}

	PlayAnimMontage(AttackMontage, 1.f, FName("Reload"));
	FTimerHandle TimerStep2;
	float ReloadDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("Reload")));

	GetWorld()->GetTimerManager().SetTimer(TimerStep2, this, &AADEnemy::Attack_ShootStep, 0.8f, false);
}
void AADEnemy::Attack_ShootStep()
{

	PlayAnimMontage(AttackMontage, 1.f, FName("Shoot"));


	float ShootDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("Shoot")));
	FTimerHandle TimerStep3;
	GetWorld()->GetTimerManager().SetTimer(TimerStep3, this, &AADEnemy::Attack_StandUpStep, ShootDuration, false);
}

void AADEnemy::Attack_StandUpStep()
{
	bIsLasing = false;
	if (LaserComp)
	{
		LaserComp->SetVisibility(false);
	}
	PlayAnimMontage(AttackMontage, 1.f, FName("StandUp"));

	float StandDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("StandUp")));
	FTimerHandle TimerStep4;
	GetWorld()->GetTimerManager().SetTimer(TimerStep4, this, &AADEnemy::Attack_End, StandDuration, false);
}

void AADEnemy::Attack_End()
{
	bIsLasing = false;
	// ★ 몽타주 강제 중단
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (AttackMontage)
			Anim->Montage_Stop(0.f, AttackMontage);
	}

	// ★ 모든 타이머 제거
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);


	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsBool("bisAttackEnd", true);
	if (LaserComp)
	{
		LaserComp->SetVisibility(false);
	}
}
void AADEnemy::DrawAimLaser()
{
	if (!TargetActor) return;

	// 총구 소켓 위치
	const FVector MuzzleLoc = MuzzlePoint->GetComponentLocation();

	// 플레이어 위치 (살짝 가슴 높이로 올리고 싶으면 Z에 +값)
	FVector TargetLoc = TargetActor->GetActorLocation();
	TargetLoc.Z += 40.f;

	// 빨간 레이저 라인
	DrawDebugLine(
		GetWorld(),
		MuzzleLoc,
		TargetLoc,
		FColor::Red,
		false,      // 영구표시 X
		0.f,        // 표시 유지 시간 (Tick마다 다시 그림)
		0,
		2.f         // 두께
	);
}
void AADEnemy::UpdateDetectState(bool _value)
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

void AADEnemy::UpdateAttackState(bool _value)
{

	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();

	BB->SetValueAsBool("bIsAttack", _value);
}

void AADEnemy::UpdateMiddleState(bool _value)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	auto BB = AI->GetBlackboardComponent();
	BB->SetValueAsBool("bIsMiddelState", _value);

}
