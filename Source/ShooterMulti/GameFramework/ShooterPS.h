#pragma once

#include "GameFramework/PlayerState.h"
#include "DeathMatchGM.h"
#include "PlayerGI.h"
#include "ShooterPS.generated.h"

UCLASS()
class SHOOTERMULTI_API AShooterPS : public APlayerState
{
	GENERATED_BODY()

protected:
	void BeginPlay() override;

public:

	UPROPERTY(Replicated, BlueprintReadOnly)
	int NbKill;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int NbDeath;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ETeam playerTeam;
	
	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	bool bIsReady = false;

	UPROPERTY(Replicated, BlueprintReadWrite, VisibleAnywhere)
	FString UserName;

	// Used to copy properties from the current PlayerState to the passed one
	virtual void CopyProperties(class APlayerState* PlayerState);
	// Used to override the current PlayerState with the properties of the passed one
	virtual void OverrideWith(class APlayerState* PlayerState);

	UFUNCTION()
	void Reset();

	void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RPCServer_PlayerIsReady(FPlayerInfo playerInfo);
	void RPCServer_PlayerIsReady_Implementation(FPlayerInfo playerInfo);

	bool isPlayersReady();
};