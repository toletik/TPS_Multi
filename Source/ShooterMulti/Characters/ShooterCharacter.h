#pragma once

#include "Net/UnrealNetwork.h"
#include "HealthCharacter.h"
#include "../Weapons/WeaponComponent.h"
#include "PlayerCameraComponent.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class EShooterCharacterState : uint8
{
	IdleRun,
	Aim,
	Sprint,
	Reload,
	Jump,
	Falling,
	Punch,
	Dead,
	PushButton
};

UCLASS()
class SHOOTERMULTI_API AShooterCharacter : public AHealthCharacter
{
	GENERATED_BODY() 

protected:

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	UWeaponComponent* Weapon;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Character|Shooter")
	UPlayerCameraComponent* Camera;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character|Shooter")
	EShooterCharacterState State;

	UPROPERTY(Replicated)
	EShooterCharacterState PrevState;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimPitch;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float AimYaw;

	void UpdateAimOffsets(float Pitch, float Yaw);

	void PlayPushButtonAnim();

	void PlayPunchAnim();

	void Falling() override;

	void BeginPlay() override;

	void Invincibility(float Duration);

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void InvincibilityFX(float Duration);
	void InvincibilityFX_Implementation(float Duration) {};

	//TEMP
	UPROPERTY(Replicated)
	FTransform _cameraTransform;

public:

	//UI
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
	FString name = "PlayerNoName";

	UPROPERTY(Replicated)
	bool bIsShooting = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float SprintSpeed = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float AimWalkSpeed = 180.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter")
	float ReloadWalkSpeed = 200.f;

	UPROPERTY(BlueprintReadOnly)
	float RunSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Shooter", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MinSprintMagnitude = .3f;

	AShooterCharacter();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	EShooterCharacterState GetState() const;
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void SetState(EShooterCharacterState InState);

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UWeaponComponent* GetWeaponComponent();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	UPlayerCameraComponent* GetCameraComponent();

	void InitPlayer();

	void InitTeamColor(ETeam InTeam);

	void Tick(float DeltaTime) override;

	UFUNCTION()
	void StartSprint();
	UFUNCTION()
	void EndSprint();

	UFUNCTION()
	void StartJump();
	UFUNCTION()
	void EndJump();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartAim();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndAim();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartShoot();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndShoot();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void StartReload();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void EndReload();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void AbortReload();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void PushButton();
	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void InflictPushButton();

	UFUNCTION(BlueprintCallable, Category = "Character|Shooter")
	void Punch();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Shooter")
	void RefreshTeamHUD(ETeam InTeam);
	void RefreshTeamHUD_Implementation(ETeam InTeam) {};

	void StartDisapear() override;
	void FinishDisapear() override;


	//Replicate
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
	
	//RPC
	UFUNCTION(Server, Reliable)
	void RPCServer_SetPrevState(EShooterCharacterState InState);
	void RPCServer_SetPrevState_Implementation(EShooterCharacterState InState);

	UFUNCTION(Server, Reliable)
	void RPCServer_SetState(EShooterCharacterState InState);
	void RPCServer_SetState_Implementation(EShooterCharacterState InState);

	UFUNCTION(Server, Reliable)
	void RPCServer_UpdateAimOffsets(float Pitch, float Yaw);
	void RPCServer_UpdateAimOffsets_Implementation(float Pitch, float Yaw);

	UFUNCTION(Server, Reliable)
	void RPCServer_Shoot(bool isShooting);
	void RPCServer_Shoot_Implementation(bool isShooting);

	UFUNCTION(Server, Reliable)
	void RPCServer_SetCameraTransform(FTransform cameraTransform = FTransform());
	void RPCServer_SetCameraTransform_Implementation(FTransform cameraTransform = FTransform());

	UFUNCTION(Server, Reliable)
	void RPCServer_PlayPunchAnim();
	void RPCServer_PlayPunchAnim_Implementation();
	UFUNCTION(NetMulticast, Reliable)
	void RPCMulticast_PlayPunchAnim();
	void RPCMulticast_PlayPunchAnim_Implementation();

	UFUNCTION(Server, Reliable)
	void RPCServer_PlayPushButtonAnim();
	void RPCServer_PlayPushButtonAnim_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void RPCMulticast_PlayPushButtonAnim();
	void RPCMulticast_PlayPushButtonAnim_Implementation();

	UFUNCTION(Server, Reliable)
	void RPCServer_SetTeam(ETeam InTeam);
	void RPCServer_SetTeam_Implementation(ETeam InTeam);

	UFUNCTION(Server, Reliable)
	void RPCServer_AddScore(class AShooterPS* shooterPlayerState);
	void RPCServer_AddScore_Implementation(class AShooterPS* shooterPlayerState);

	UFUNCTION(Server, Reliable)
	void RPCServer_AddNumberOfDeath(class AShooterPS* shooterPlayerState);
	void RPCServer_AddNumberOfDeath_Implementation(class AShooterPS* shooterPlayerState);

	UFUNCTION(Server, Reliable)
	void RPCServer_SetName();
	void RPCServer_SetName_Implementation();

	UFUNCTION(Server, Reliable)
	void RPCServer_RemoveAI();
	void RPCServer_RemoveAI_Implementation();
};