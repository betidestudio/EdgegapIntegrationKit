// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ListAllDeployments.generated.h"

/**
 * Filter operator for deployment list queries
 */
UENUM(BlueprintType)
enum class EEGIK_DeploymentFilterOperator : uint8
{
	Equal UMETA(DisplayName = "eq"),
	NotEqual UMETA(DisplayName = "neq"),
	In UMETA(DisplayName = "in"),
	NotIn UMETA(DisplayName = "nin"),
	LessThan UMETA(DisplayName = "lt"),
	LessThanOrEqual UMETA(DisplayName = "lte"),
	GreaterThan UMETA(DisplayName = "gt"),
	GreaterThanOrEqual UMETA(DisplayName = "gte")
};

/**
 * Sort direction for deployment list queries
 */
UENUM(BlueprintType)
enum class EEGIK_SortDirection : uint8
{
	Ascending UMETA(DisplayName = "asc"),
	Descending UMETA(DisplayName = "desc")
};

/**
 * A single filter for deployment list queries.
 * Fields: status, request_id, tags, created_at, application, version, is_joinable_by_session, available_session_sockets
 */
USTRUCT(BlueprintType)
struct FEGIK_DeploymentFilter
{
	GENERATED_BODY()

	// The field to filter on (e.g., "status", "application", "tags", "version", "created_at")
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Field;

	// The comparison operator
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	EEGIK_DeploymentFilterOperator Operator = EEGIK_DeploymentFilterOperator::Equal;

	// The value to filter by (single value for eq/neq/lt/lte/gt/gte, ignored if using ArrayValues)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Value;

	// Array of values for in/nin operators
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> ArrayValues;
};

/**
 * Sort order for deployment list queries.
 * Fields: created_at, available_session_sockets
 */
USTRUCT(BlueprintType)
struct FEGIK_DeploymentSortOrder
{
	GENERATED_BODY()

	// The field to sort by (e.g., "created_at", "available_session_sockets")
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Field = TEXT("created_at");

	// Sort direction
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	EEGIK_SortDirection Direction = EEGIK_SortDirection::Descending;
};

/**
 * Query parameters for listing deployments
 */
USTRUCT(BlueprintType)
struct FEGIK_ListDeploymentsQuery
{
	GENERATED_BODY()

	// Filters to apply
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_DeploymentFilter> Filters;

	// Sort order
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_DeploymentSortOrder> OrderBy;

	// Page number (1-based)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Page = 1;

	// Results per page
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Limit = 20;
};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentListData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString FQDN;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bReady = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString PublicIP;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TMap<FString, FEGIK_DeploymentPorts> Ports;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Sockets = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 SocketsUsage = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bIsJoinableBySession = false;
};

USTRUCT(BlueprintType)
struct FEGIK_Pagination
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Number = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 NextPageNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 PreviousPageNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 TotalPages = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bHasNextPage = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bHasPreviousPage = false;
};

USTRUCT(BlueprintType)
struct FEGIK_ListAllDeploymentsResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_DeploymentListData> Deployments;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 TotalCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_Pagination Pagination;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Message;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_ListAllDeploymentsDelegate, const FEGIK_ListAllDeploymentsResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListAllDeployments : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_ListAllDeployments* ListAllDeployments(FEGIK_ListDeploymentsQuery Query);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_ListAllDeploymentsDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_ListAllDeploymentsDelegate OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FEGIK_ListDeploymentsQuery Var_Query;

	static FString FilterOperatorToString(EEGIK_DeploymentFilterOperator Op);
	static FString SortDirectionToString(EEGIK_SortDirection Dir);
	static TArray<FString> JsonArrayToStringArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray);
};
