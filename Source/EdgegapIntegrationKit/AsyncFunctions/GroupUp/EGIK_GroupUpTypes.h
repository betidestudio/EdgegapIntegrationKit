// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_GroupUpTypes.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_GroupUpGroupResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString TeamId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString Status;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_AssignmentStruct Assignment;

	FEGIK_GroupUpGroupResponse()
	{
		TeamId = TEXT("");
		Status = TEXT("");
		Assignment = FEGIK_AssignmentStruct(TEXT("null"));
	}

	explicit FEGIK_GroupUpGroupResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		TeamId = TEXT("");
		Status = TEXT("");
		Assignment = FEGIK_AssignmentStruct(TEXT("null"));

		if (!JsonObject.IsValid())
		{
			return;
		}

		JsonObject->TryGetStringField(TEXT("team_id"), TeamId);
		JsonObject->TryGetStringField(TEXT("status"), Status);

		const TSharedPtr<FJsonObject>* AssignmentObject;
		if (JsonObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
		{
			Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
		}
	}
};

USTRUCT(BlueprintType)
struct FEGIK_GroupUpMemberResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString MemberId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString GroupId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	bool bIsReady = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString TeamId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString Status;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_AssignmentStruct Assignment;

	FEGIK_GroupUpMemberResponse()
	{
		MemberId = TEXT("");
		GroupId = TEXT("");
		TicketId = TEXT("");
		TeamId = TEXT("");
		Status = TEXT("");
		Assignment = FEGIK_AssignmentStruct(TEXT("null"));
	}

	explicit FEGIK_GroupUpMemberResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		MemberId = TEXT("");
		GroupId = TEXT("");
		bIsReady = false;
		TicketId = TEXT("");
		TeamId = TEXT("");
		Status = TEXT("");
		Assignment = FEGIK_AssignmentStruct(TEXT("null"));

		if (!JsonObject.IsValid())
		{
			return;
		}

		JsonObject->TryGetStringField(TEXT("member_id"), MemberId);
		JsonObject->TryGetStringField(TEXT("group_id"), GroupId);
		JsonObject->TryGetBoolField(TEXT("is_ready"), bIsReady);
		JsonObject->TryGetStringField(TEXT("ticket_id"), TicketId);
		JsonObject->TryGetStringField(TEXT("team_id"), TeamId);
		JsonObject->TryGetStringField(TEXT("status"), Status);

		const TSharedPtr<FJsonObject>* AssignmentObject;
		if (JsonObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
		{
			Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
		}
	}
};
