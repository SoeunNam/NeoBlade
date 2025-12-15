// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneEnemy.h"

#include "DroneEnemyFSM.h"
#include "PlayerCharacter.h"

#include <Components/CapsuleComponent.h>
#include <Components/SphereComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>


// Sets default values
ADroneEnemy::ADroneEnemy()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    /*ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"));
    if (tempMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(tempMesh.Object);
        GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -88), FRotator(0, -90, 0));
        GetMesh()->SetRelativeScale3D(FVector(0.84f));
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }*/

    sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collosion"));
    sphereComp->SetRelativeScale3D(FVector(3, 3, 3));
    sphereComp->SetCollisionProfileName(TEXT("EnemyHand"));
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
    GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

    fsm = CreateDefaultSubobject<UDroneEnemyFSM>(TEXT("FSM"));
    if (fsm) {
        UE_LOG(LogTemp, Warning, TEXT("fsm"));

    }
    //ConstructorHelpers::FClassFinder<UAnimInstance> tempClass(TEXT("/Game/Anim/ABP_Enemy.ABP_Enemy_C"));
    
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void ADroneEnemy::BeginPlay()
{
    Super::BeginPlay();
    auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerCharacter::StaticClass());

    target = Cast<APlayerCharacter>(actor);
    me = this;
    /*
    widgetComp->InitWidget();
    hpBar = Cast<UHPBar>(widgetComp->GetUserWidgetObject());
    if (IsValid(hpBar)) {
        //FText idText = FText::FromString(FString::FromInt(12));
        FVector _pos = target->GetActorLocation();
        hpBar->ctextId = GetClass()->GetDisplayNameText();
        hpBar->cInfo = FText::FromString(FString::Printf(TEXT("IDLE")));
        //hpBar->ctextPos = FText::FromString(FString::Printf(TEXT("%f %f %f"), _pos.X, _pos.Y, _pos.Z));
        hpBar->cHP = 10;
        hpBar->cMaxHP = 10;
        hpBar->cMP = 100;
        hpBar->cMaxMP = 100;

        widget->SetTextID(GetClass()->GetDisplayNameText());
        FVector _pos = target->GetActorLocation();
        widget->SetTextPosition(FText::FromString(FString::Printf(TEXT("%f %f %f"), _pos.X, _pos.Y, _pos.Z)));
        
    }
    */
}

// Called every frame
void ADroneEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADroneEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}

/*
void ADroneEnemy::Attack()
{
    Super::Attack();
    Fire();
}

void ADroneEnemy::Fire()
{
    if (!BulletClass) return;
    // 캐릭터 메시의 특정 Bone 위치에서 총알 생성
    FVector Start = GetMesh()->GetBoneLocation("pistol_cylinder", EBoneSpaces::WorldSpace);
    FRotator SpawnRotation = GetActorRotation();

    if (ACharacter* Player = Cast<ACharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
    {
        // 총알 생성 지점과 플레이어 위치를 통해 방향 벡터 구하고 정규화
        FVector Direction = (Player->GetActorLocation() - Start).GetSafeNormal();
        //DrawDebugLine(GetWorld(),Start,Start+Direction*5000,FColor::Red,false,1.0f,0,2.0f);
        //총알 생성
        if (ABaseBullet* Bullet = GetWorld()->SpawnActor<ABaseBullet>(BulletClass, Start, SpawnRotation))
        {
            //총알 데미지 설정
            Bullet->SetBulletDamage(Power);
            if (UProjectileMovementComponent* MovementComp = Bullet->GetProjectileComp())
            {
                //총알이 Velocity 방향을 따르도록 설정 및 Velocity 값 방향벡터와 속도 이용해 설정
                MovementComp->bRotationFollowsVelocity = true;
                MovementComp->Velocity = Direction * MovementComp->GetMaxSpeed();
            }
        }
    }
}
*/