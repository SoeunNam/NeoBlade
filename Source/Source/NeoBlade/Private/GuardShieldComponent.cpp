#include "GuardShieldComponent.h"
#include "PlayerCharacter.h"
#include "PlayerCombatComponent.h"
#include "Components/StaticMeshComponent.h"

UGuardShieldComponent::UGuardShieldComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // ★ 정석: 서브오브젝트로 생성해야 Attach가 정상 작동함
    ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
    ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldMesh->SetVisibility(false);
    ShieldMesh->SetGenerateOverlapEvents(false);
}

void UGuardShieldComponent::BeginPlay()
{
    Super::BeginPlay();

    APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("[Shield] Player nullptr"));
        return;
    }

    // ★ 플레이어 Mesh 에 소켓으로 Attach
    ShieldMesh->AttachToComponent(
        Player->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("ShieldSocket")
    );


    // ★ 머터리얼을 MID로 변환해야 ShieldPower 파라미터 사용 가능
    ShieldMesh->CreateAndSetMaterialInstanceDynamic(0);

    ShieldMesh->SetWorldScale3D(FVector(2.0f));

}

bool UGuardShieldComponent::CanActivateShield() const
{
    return !bShieldOnCooldown && CurrentShield > 0;
}

void UGuardShieldComponent::ActivateShield()
{
    if (ShieldMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shield] SHOW"));
        ShieldMesh->SetVisibility(true, true);
    }
}

void UGuardShieldComponent::DeactivateShield()
{
    if (ShieldMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shield] HIDE"));
        ShieldMesh->SetVisibility(false, true);
    }
}

bool UGuardShieldComponent::OnGuardHit(AActor* Attacker)
{
    if (CurrentShield <= 0)
        return false;

    CurrentShield--;

    float Alpha = (float)CurrentShield / (float)MaxShield;
    ShieldMesh->SetScalarParameterValueOnMaterials(TEXT("ShieldPower"), Alpha);

    if (CurrentShield <= 0)
    {
        DeactivateShield();
        StartCooldown();

        if (auto* Combat = GetOwner()->FindComponentByClass<UPlayerCombatComponent>())
            Combat->EndGuard();

        return false;
    }

    return true;
}

void UGuardShieldComponent::StartCooldown()
{
    bShieldOnCooldown = true;

    GetWorld()->GetTimerManager().SetTimer(
        ShieldCooldownTimer,
        this,
        &UGuardShieldComponent::ResetShield,
        ShieldCooldown,
        false
    );
}

void UGuardShieldComponent::ResetShield()
{
    CurrentShield = MaxShield;
    bShieldOnCooldown = false;
}
