// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_ListAllDeployments.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_ListAllDeployments* UEGIK_ListAllDeployments::ListAllDeployments(FEGIK_ListDeploymentsQuery Query)
{
	UEGIK_ListAllDeployments* Node = NewObject<UEGIK_ListAllDeployments>();
	Node->Var_Query = Query;
	return Node;
}

FString UEGIK_ListAllDeployments::GetEndpointURL() const
{
	FString URL = TEXT("https://api.edgegap.com/v1/deployments");
	FString QueryParams;

	// Build the JSON query object if filters or order_by are specified
	if (Var_Query.Filters.Num() > 0 || Var_Query.OrderBy.Num() > 0)
	{
		TSharedPtr<FJsonObject> QueryObject = MakeShareable(new FJsonObject);

		// Build filters array
		if (Var_Query.Filters.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> FiltersArray;
			for (const auto& Filter : Var_Query.Filters)
			{
				TSharedPtr<FJsonObject> FilterObj = MakeShareable(new FJsonObject);
				FilterObj->SetStringField(TEXT("field"), Filter.Field);
				FilterObj->SetStringField(TEXT("operator"), FilterOperatorToString(Filter.Operator));

				// Use array value for in/nin operators
				if (Filter.Operator == EEGIK_DeploymentFilterOperator::In || Filter.Operator == EEGIK_DeploymentFilterOperator::NotIn)
				{
					TArray<TSharedPtr<FJsonValue>> ValuesArray;
					for (const FString& Val : Filter.ArrayValues)
					{
						ValuesArray.Add(MakeShareable(new FJsonValueString(Val)));
					}
					FilterObj->SetArrayField(TEXT("value"), ValuesArray);
				}
				else
				{
					FilterObj->SetStringField(TEXT("value"), Filter.Value);
				}

				FiltersArray.Add(MakeShareable(new FJsonValueObject(FilterObj)));
			}
			QueryObject->SetArrayField(TEXT("filters"), FiltersArray);
		}

		// Build order_by array
		if (Var_Query.OrderBy.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> OrderByArray;
			for (const auto& Order : Var_Query.OrderBy)
			{
				TSharedPtr<FJsonObject> OrderObj = MakeShareable(new FJsonObject);
				OrderObj->SetStringField(TEXT("field"), Order.Field);
				OrderObj->SetStringField(TEXT("order"), SortDirectionToString(Order.Direction));
				OrderByArray.Add(MakeShareable(new FJsonValueObject(OrderObj)));
			}
			QueryObject->SetArrayField(TEXT("order_by"), OrderByArray);
		}

		FString QueryJsonString = JsonObjectToString(QueryObject);
		QueryParams += TEXT("query=") + FGenericPlatformHttp::UrlEncode(QueryJsonString);
	}

	// Add pagination
	if (Var_Query.Page > 1)
	{
		if (!QueryParams.IsEmpty()) QueryParams += TEXT("&");
		QueryParams += FString::Printf(TEXT("page=%d"), Var_Query.Page);
	}

	if (Var_Query.Limit != 20)
	{
		if (!QueryParams.IsEmpty()) QueryParams += TEXT("&");
		QueryParams += FString::Printf(TEXT("limit=%d"), Var_Query.Limit);
	}

	if (!QueryParams.IsEmpty())
	{
		URL += TEXT("?") + QueryParams;
	}

	return URL;
}

EEGIK_HttpVerb UEGIK_ListAllDeployments::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_ListAllDeployments::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_ListAllDeploymentsResponse Response;

	// Parse Pagination
	if (JsonObject->HasField(TEXT("pagination")))
	{
		TSharedPtr<FJsonObject> PaginationObject = JsonObject->GetObjectField(TEXT("pagination"));
		Response.Pagination.Number = PaginationObject->GetIntegerField(TEXT("number"));
		Response.Pagination.NextPageNumber = PaginationObject->GetIntegerField(TEXT("next_page_number"));
		Response.Pagination.PreviousPageNumber = PaginationObject->GetIntegerField(TEXT("previous_page_number"));

		if (PaginationObject->HasField(TEXT("total_pages")))
		{
			Response.Pagination.TotalPages = PaginationObject->GetIntegerField(TEXT("total_pages"));
		}
		else if (PaginationObject->HasField(TEXT("paginator")))
		{
			TSharedPtr<FJsonObject> Paginator = PaginationObject->GetObjectField(TEXT("paginator"));
			if (Paginator->HasField(TEXT("num_pages")))
			{
				Response.Pagination.TotalPages = Paginator->GetIntegerField(TEXT("num_pages"));
			}
		}

		Response.Pagination.bHasNextPage = PaginationObject->GetBoolField(TEXT("has_next"));
		Response.Pagination.bHasPreviousPage = PaginationObject->GetBoolField(TEXT("has_previous"));
	}

	// Parse Total Count
	if (JsonObject->HasField(TEXT("total_count")))
	{
		Response.TotalCount = JsonObject->GetIntegerField(TEXT("total_count"));
	}

	// Parse Data Array
	const TArray<TSharedPtr<FJsonValue>>* DataArrayPtr;
	if (JsonObject->TryGetArrayField(TEXT("data"), DataArrayPtr) && DataArrayPtr)
	{
		for (const TSharedPtr<FJsonValue>& Val : *DataArrayPtr)
		{
			if (!Val.IsValid()) continue;

			TSharedPtr<FJsonObject> DeploymentObject = Val->AsObject();
			if (!DeploymentObject.IsValid()) continue;

			FEGIK_DeploymentListData DeploymentData;
			DeploymentData.RequestId = DeploymentObject->GetStringField(TEXT("request_id"));
			DeploymentData.FQDN = DeploymentObject->GetStringField(TEXT("fqdn"));
			DeploymentData.StartTime = DeploymentObject->GetStringField(TEXT("start_time"));
			DeploymentData.bReady = DeploymentObject->GetBoolField(TEXT("ready"));
			DeploymentData.PublicIP = DeploymentObject->GetStringField(TEXT("public_ip"));

			if (DeploymentObject->HasField(TEXT("sockets")))
			{
				DeploymentData.Sockets = DeploymentObject->GetIntegerField(TEXT("sockets"));
			}

			if (DeploymentObject->HasField(TEXT("sockets_usage")))
			{
				DeploymentData.SocketsUsage = DeploymentObject->GetIntegerField(TEXT("sockets_usage"));
			}

			DeploymentData.bIsJoinableBySession = DeploymentObject->GetBoolField(TEXT("is_joinable_by_session"));

			// Parse Tags
			const TArray<TSharedPtr<FJsonValue>>* TagsArrayPtr;
			if (DeploymentObject->TryGetArrayField(TEXT("tags"), TagsArrayPtr))
			{
				DeploymentData.Tags = JsonArrayToStringArray(TagsArrayPtr);
			}

			// Parse Ports
			const TSharedPtr<FJsonObject>* PortsObjectPtr;
			if (DeploymentObject->TryGetObjectField(TEXT("ports"), PortsObjectPtr) && PortsObjectPtr)
			{
				TSharedPtr<FJsonObject> PortsObject = *PortsObjectPtr;
				for (const auto& PortEntry : PortsObject->Values)
				{
					TSharedPtr<FJsonObject> PortObject = PortEntry.Value->AsObject();
					if (!PortObject.IsValid()) continue;

					FEGIK_DeploymentPorts PortData;
					PortObject->TryGetNumberField(TEXT("external"), PortData.External);
					PortObject->TryGetNumberField(TEXT("internal"), PortData.Internal);
					PortObject->TryGetStringField(TEXT("protocol"), PortData.Protocol);
					PortObject->TryGetBoolField(TEXT("tls_upgrade"), PortData.bTLS_Upgrade);
					PortObject->TryGetStringField(TEXT("link"), PortData.Link);
					PortObject->TryGetNumberField(TEXT("proxy"), PortData.Proxy);

					DeploymentData.Ports.Add(PortEntry.Key, PortData);
				}
			}

			Response.Deployments.Add(DeploymentData);
		}
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_ListAllDeployments::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ListAllDeploymentsResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

FString UEGIK_ListAllDeployments::FilterOperatorToString(EEGIK_DeploymentFilterOperator Op)
{
	switch (Op)
	{
	case EEGIK_DeploymentFilterOperator::Equal: return TEXT("eq");
	case EEGIK_DeploymentFilterOperator::NotEqual: return TEXT("neq");
	case EEGIK_DeploymentFilterOperator::In: return TEXT("in");
	case EEGIK_DeploymentFilterOperator::NotIn: return TEXT("nin");
	case EEGIK_DeploymentFilterOperator::LessThan: return TEXT("lt");
	case EEGIK_DeploymentFilterOperator::LessThanOrEqual: return TEXT("lte");
	case EEGIK_DeploymentFilterOperator::GreaterThan: return TEXT("gt");
	case EEGIK_DeploymentFilterOperator::GreaterThanOrEqual: return TEXT("gte");
	default: return TEXT("eq");
	}
}

FString UEGIK_ListAllDeployments::SortDirectionToString(EEGIK_SortDirection Dir)
{
	switch (Dir)
	{
	case EEGIK_SortDirection::Ascending: return TEXT("asc");
	case EEGIK_SortDirection::Descending: return TEXT("desc");
	default: return TEXT("desc");
	}
}

TArray<FString> UEGIK_ListAllDeployments::JsonArrayToStringArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray)
{
	TArray<FString> StringArray;
	if (JsonArray)
	{
		for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
		{
			if (Value->Type == EJson::String)
			{
				StringArray.Add(Value->AsString());
			}
		}
	}
	return StringArray;
}
