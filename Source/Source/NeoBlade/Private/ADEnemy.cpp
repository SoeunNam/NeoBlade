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
#include "Components/CapsuleComponent.h"
#include"RoomManager.h"

// Sets default values
AADEnemy::AADEnemy()
{
	HealthComp = CreateDefaultSubobject<UNomal_EnemyHealthComponent>(TEXT("HealthComponent"));
	DamageComp = CreateDefaultSubobject<UNomal_EnemyDamageComponent>(TEXT("DamageComponent"));
	HitReactionComp = CreateDefaultSubobject<UNomal_EnemyHitReactionComponent>(TEXT("HitReactionComponent"));

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(RootComponent);   // 나중에 소켓에 붙이고 싶으면 여기서 변경

	LaserComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LaserComp"));
	LaserComp->SetupAttachment(MuzzlePoint);       // 총구에 붙이기
	LaserComp->SetAutoActivate(false);
	LaserComp->SetVisibility(false);
}

// Called when the game starts or when spawned
void AADEnemy::BeginPlay()
{
	Super::BeginPlay();


	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsObject("TargetActor", TargetActor);
		}
	}

	// 애니 인스턴스 바인딩
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		Anim->OnMontageEnded.AddDynamic(this, &AADEnemy::OnMontageEnded);
	}

	// Niagara 이펙트 연결 (옵션)
	if (LaserEffect && LaserComp)
	{
		LaserComp->SetAsset(LaserEffect);   // 템플릿 지정
		LaserComp->SetVisibility(false);
		LaserComp->DeactivateImmediate();
	}
	//if (LaserEffect)
	//{
	//	
	//	LaserComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
	//		LaserEffect,
	//		MuzzlePoint,
	//		NAME_None,
	//		FVector::ZeroVector,
	//		FRotator::ZeroRotator,
	//		EAttachLocation::SnapToTarget,
	//		true  // 처음엔 꺼둠
	//	);
	//	if (LaserComp)
	//	{
	//		LaserComp->SetVisibility(false);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("❌ BeginPlay: LaserComp Spawn Failed (%s)"), *GetName());
	//	}
	//}
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

		/*DrawAimLaser();*/
	}
	if (bIsLasing && LaserComp)
	{
		UpdateLaserEffect();
	}
	else if (LaserComp)
	{
		LaserComp->SetVisibility(false);
	}
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
	if (bIsDead) return;


	if (!LaserComp || !TargetActor) return;

	FVector Start = MuzzlePoint
		? MuzzlePoint->GetComponentLocation()
		: GetActorLocation();

	FVector End = TargetActor->GetActorLocation();
	End.Z += 40.f;

	LaserComp->SetVisibility(true);
	LaserComp->Activate(true);


	// ✅ Niagara에서 만든 "진짜 이름" 그대로 써야 한다
	LaserComp->SetVariableVec3(TEXT("User_Start"), Start);
	LaserComp->SetVariableVec3(TEXT("End"), End);

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
		UpdateAttackState(false);
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

void AADEnemy::A_PlayAirAttackedHitMontage()
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

void AADEnemy::A_PlayAirBorneHitMontage()
{
	bisAirborne = true;
	Attack_End();
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
void AADEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	FTimerHandle SetAirborne;
	float HitDuration = AirBorneMontage ? AirBorneMontage->GetPlayLength() : 0.5f;

	GetWorld()->GetTimerManager().SetTimer(SetAirborne, [&]()
		{

			bIsMiddleState = false;
			UpdateMiddleState(false);
			/*UpdateAttackState(false);*/
		}, 1.0f, false);

}
void AADEnemy::A_PlayFinalAttackHitMontage()
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
		GetMesh()->GetAnimInstance()->Montage_Play(FinalAttackMontage);

		// 2) 중간 상태 업데이트
		UpdateMiddleState(true);

	}
}

void AADEnemy::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("✅ AADEnemy::Die() Start: %s"), *GetName());

	// ✅ 1. LaserComp 안전 처리
	if (LaserComp)
	{
		LaserComp->SetVisibility(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Die(): LaserComp is NULL"));
	}

	// ✅ 2. Mesh / Anim 안전 처리
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Die(): Mesh is NULL"));
		return;
	}

	UAnimInstance* Anim = MeshComp->GetAnimInstance();
	if (!Anim)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Die(): AnimInstance is NULL"));
	}
	else
	{
		if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
		{
			Anim->Montage_Stop(0.1f, AttackMontage);
		}

		if (DeathMontage)
		{
			Anim->Montage_Play(DeathMontage);
		}
	}

	// ✅ 3. 메시 콜리전 제거 (시체는 통과)
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

	// ✅ 4. AI / Blackboard 안전 처리
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsBool("IsDead", true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("❌ Die(): Blackboard is NULL"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Die(): AIController is NULL"));
	}

	// ✅ 5. RoomManager 안전 처리
	TArray<AActor*> Rooms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomManager::StaticClass(), Rooms);

	if (Rooms.Num() > 0)
	{
		if (ARoomManager* Room = Cast<ARoomManager>(Rooms[0]))
		{
			Room->OnEnemyDead();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Die(): No RoomManager Found"));
	}
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
{    // ✅ UE5 호환 생존 체크
	if (!IsValid(this))
		return;

	if (!IsValid(TargetActor))
		return;

	if (bIsattack)
		return;

	// ✅ 애니메이션 안전 체크
	if (GetMesh())
	{
		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst && AttackMontage)
		{
			if (AnimInst->Montage_IsPlaying(AttackMontage))
				return;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector PlayerPos = TargetActor->GetActorLocation();

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
		return;

	const ANavigationData* NavData =
		NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!NavData)
		return;

	FNavLocation Result;
	bool bInNavMesh = NavSys->ProjectPointToNavigation(
		PlayerPos,
		Result,
		FVector(50.f, 50.f, 200.f)
	);

	UpdateDetectState(bInNavMesh);
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
	UE_LOG(LogTemp, Warning, TEXT("✅ AADEnemy::Attack() ENTER"));

	UE_LOG(LogTemp, Warning, TEXT("bIsDead = %d"), bIsDead);
	UE_LOG(LogTemp, Warning, TEXT("AttackMontage = %s"),
		AttackMontage ? TEXT("VALID") : TEXT("NULL"));

	if (!GetMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Mesh is NULL"));
		return;
	}

	if (!GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ AnimInstance is NULL"));
		return;
	}

	bool bPlaying = GetMesh()->GetAnimInstance()->Montage_IsPlaying(AttackMontage);
	UE_LOG(LogTemp, Warning, TEXT("IsPlayingAttackMontage = %d"), bPlaying);

	AAIController* AI = Cast<AAIController>(GetController());
	UE_LOG(LogTemp, Warning, TEXT("AIController = %s"),
		AI ? TEXT("VALID") : TEXT("NULL"));

	if (bIsDead || !AttackMontage || bPlaying || !AI)
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Attack BLOCKED by condition"));
		return;
	}

	// ✅ 여기까지 오면 “공격 조건 통과”
	UE_LOG(LogTemp, Warning, TEXT("✅ Attack 조건 통과, 실제 공격 시작"));


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
	if (bIsDead) return;
	PlayAnimMontage(AttackMontage, 1.f, FName("Reload"));
	FTimerHandle TimerStep2;
	float ReloadDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("Reload")));

	GetWorld()->GetTimerManager().SetTimer(TimerStep2, this, &AADEnemy::Attack_ShootStep, 0.8f, false);
}
void AADEnemy::Attack_ShootStep()
{
	if (bIsDead) return;
	PlayAnimMontage(AttackMontage, 1.f, FName("Shoot"));


	float ShootDuration = AttackMontage->GetSectionLength(AttackMontage->GetSectionIndex(FName("Shoot")));
	FTimerHandle TimerStep3;
	GetWorld()->GetTimerManager().SetTimer(TimerStep3, this, &AADEnemy::Attack_StandUpStep, ShootDuration, false);
}

void AADEnemy::Attack_StandUpStep()
{
	if (bIsDead) return;
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
	if (bIsDead) return;
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
	if (bIsDead) return;
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
