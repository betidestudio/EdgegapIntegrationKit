// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "APITokenSettings.generated.h"

USTRUCT(BlueprintType)
struct FAPITokenSettings
{
	GENERATED_BODY()

public:
	// API Token
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "API Token")
	FString APIToken;
};

