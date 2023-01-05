#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cube.generated.h"

UCLASS()
class SHOOTERMULTI_API ACube : public APawn
{
	GENERATED_BODY()
	
public:	
	ACube();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
