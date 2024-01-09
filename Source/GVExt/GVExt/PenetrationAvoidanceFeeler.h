// Copyright (C) 2024 owoDra

#pragma once

#include "PenetrationAvoidanceFeeler.generated.h"


/**
 * Struct defining a feeler ray used for camera penetration avoidance.
 */
USTRUCT()
struct FPenetrationAvoidanceFeeler
{
	GENERATED_BODY()
public:
	FPenetrationAvoidanceFeeler()
		: AdjustmentRot(ForceInit)
		, WorldWeight(0)
		, PawnWeight(0)
		, Extent(0)
		, TraceInterval(0)
		, FramesUntilNextTrace(0)
	{
	}

	FPenetrationAvoidanceFeeler(
		const FRotator& InAdjustmentRot,
		const float& InWorldWeight,
		const float& InPawnWeight,
		const float& InExtent,
		const int32& InTraceInterval = 0,
		const int32& InFramesUntilNextTrace = 0
	)
		: AdjustmentRot(InAdjustmentRot)
		, WorldWeight(InWorldWeight)
		, PawnWeight(InPawnWeight)
		, Extent(InExtent)
		, TraceInterval(InTraceInterval)
		, FramesUntilNextTrace(InFramesUntilNextTrace)
	{
	}

public:
	//
	// describing deviance from main ray
	//
	UPROPERTY(EditAnywhere)
	FRotator AdjustmentRot;

	//
	// How much this feeler affects the final position if it hits the world
	//
	UPROPERTY(EditAnywhere)
	float WorldWeight;

	//
	// how much this feeler affects the final position if it hits a APawn 
	// (setting to 0 will not attempt to collide with pawns at all)
	//
	UPROPERTY(EditAnywhere)
	float PawnWeight;

	//
	// Extent to use for collision when tracing this feeler
	//
	UPROPERTY(EditAnywhere)
	float Extent;

	//
	// Minimum frame interval between traces with this feeler if nothing was hit last frame
	//
	UPROPERTY(EditAnywhere)
	int32 TraceInterval;

	//
	// Number of frames since this feeler was used
	//
	UPROPERTY(EditAnywhere)
	int32 FramesUntilNextTrace;

};