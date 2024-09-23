// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_ListRelaySessions.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_ListRelayPagination
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	int32 Number;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	int32 NextPageNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	int32 PreviousPageNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	int32 TotalPages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	bool bHasNextPage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	bool bHasPreviousPage;
};

USTRUCT(BlueprintType)
struct FEGIK_ListRelaySessionsOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	TArray<FEGIK_RelaySessionInfo> RelaySessions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	FEGIK_ListRelayPagination Pagination;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListRelaySessions, FEGIK_ListRelaySessionsOutput, RelaySessions, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListRelaySessions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Relay Sessions", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_ListRelaySessions* ListRelaySessions(int32 PageNumber);

	virtual void Activate() override;
	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnListRelaySessions OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnListRelaySessions OnFailure;

private:
	int32 Var_PageNumber;
};
