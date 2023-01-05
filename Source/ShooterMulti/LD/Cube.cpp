#include "Cube.h"

ACube::ACube()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
}

void ACube::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

