// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetDeploymentMetrics.h"

UEGIK_GetDeploymentMetrics* UEGIK_GetDeploymentMetrics::GetDeploymentMetrics(const FString& RequestId)
{
	UEGIK_GetDeploymentMetrics* Node = NewObject<UEGIK_GetDeploymentMetrics>();
	Node->Var_RequestId = RequestId;
	return Node;
}

FString UEGIK_GetDeploymentMetrics::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/metrics/deployment/%s"), *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_GetDeploymentMetrics::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetDeploymentMetrics::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_DeploymentMetricsResponse Response;
	Response.RequestId = Var_RequestId;

	if (JsonObject.IsValid())
	{
		// Get timestamp
		if (JsonObject->HasField(TEXT("timestamp")))
		{
			Response.Timestamp = JsonObject->GetStringField(TEXT("timestamp"));
		}

		// Parse CPU metrics
		if (JsonObject->HasField(TEXT("cpu")))
		{
			TSharedPtr<FJsonObject> CpuObject = JsonObject->GetObjectField(TEXT("cpu"));
			if (CpuObject.IsValid())
			{
				Response.Cpu.UsagePercent = CpuObject->GetNumberField(TEXT("usage_percent"));
				Response.Cpu.LimitMillicores = CpuObject->GetIntegerField(TEXT("limit_millicores"));
				Response.Cpu.UsageMillicores = CpuObject->GetIntegerField(TEXT("usage_millicores"));
			}
		}

		// Parse Memory metrics
		if (JsonObject->HasField(TEXT("memory")))
		{
			TSharedPtr<FJsonObject> MemoryObject = JsonObject->GetObjectField(TEXT("memory"));
			if (MemoryObject.IsValid())
			{
				Response.Memory.UsagePercent = MemoryObject->GetNumberField(TEXT("usage_percent"));
				Response.Memory.LimitBytes = static_cast<int64>(MemoryObject->GetNumberField(TEXT("limit_bytes")));
				Response.Memory.UsageBytes = static_cast<int64>(MemoryObject->GetNumberField(TEXT("usage_bytes")));
			}
		}

		// Parse Network metrics
		if (JsonObject->HasField(TEXT("network")))
		{
			TSharedPtr<FJsonObject> NetworkObject = JsonObject->GetObjectField(TEXT("network"));
			if (NetworkObject.IsValid())
			{
				Response.Network.RxBytes = static_cast<int64>(NetworkObject->GetNumberField(TEXT("rx_bytes")));
				Response.Network.TxBytes = static_cast<int64>(NetworkObject->GetNumberField(TEXT("tx_bytes")));
				Response.Network.RxPackets = static_cast<int64>(NetworkObject->GetNumberField(TEXT("rx_packets")));
				Response.Network.TxPackets = static_cast<int64>(NetworkObject->GetNumberField(TEXT("tx_packets")));
			}
		}
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_GetDeploymentMetrics::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_DeploymentMetricsResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
