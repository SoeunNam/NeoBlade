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
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 700.f, 0.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCharacterMovement()->bEnablePhysicsInteraction = false;   // 물리 반응 끔
	GetCharacterMovement()->PushForceFactor = 0.0f;              // 밀림 제거
	GetCharacterMovement()->MaxDepenetrationWithPawn = 0.0f;     // 겹침 보정 제거
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

	BB->SetValueAsObject("EnemyActor", TargetActor);



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

	// 새로운 상태 계산
	bool bNewAttack = (DistSq < AttackSq);


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

	// Attack Range 업데이트
	if (bNewAttack == true)
	{
		bIsInAttackRange = bNewAttack;
		UpdateAttackState(bIsInAttackRange);
	}


	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (bIsAttacking && Anim && !Anim->Montage_IsPlaying(AttackMontage))
	{
		// -> 강제 AttackEnd 처리
		ForceAttackEnd();
	}
	CheckPlayerInNavMesh();
	DrawDebugDirectionalArrow(
		GetWorld(),
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * 200,
		20,
		FColor::Red,
		false,
		-1,
		0,
		2
	);
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

void AMeleeEnemy::Die()
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
