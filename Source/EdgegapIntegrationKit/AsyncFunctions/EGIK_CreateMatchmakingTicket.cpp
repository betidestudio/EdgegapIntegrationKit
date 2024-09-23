// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_CreateMatchmakingTicket.h"

UEGIK_CreateMatchmakingTicket* UEGIK_CreateMatchmakingTicket::CreateMatchmakingTicket(const FEGIK_CreateMatchmakingStruct& MatchmakingStruct)
{
	UEGIK_CreateMatchmakingTicket* BlueprintNode = NewObject<UEGIK_CreateMatchmakingTicket>();
	BlueprintNode->Var_MatchmakingStruct = MatchmakingStruct;
	return BlueprintNode;
}

void UEGIK_CreateMatchmakingTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	FEGIK_MatchmakingResponse Response;
	if(HttpResponse.IsValid())
	{
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				Response.TicketId = JsonObject->GetStringField(TEXT("id"));
				Response.GameProfile = JsonObject->GetStringField(TEXT("game_profile"));
				FDateTime::ParseIso8601(*JsonObject->GetStringField(TEXT("created_at")), Response.CreatedAt);
				const TSharedPtr<FJsonObject>* AssignmentObject;
				if (JsonObject->HasTypedField<EJson::Object>(TEXT("assignment")))
				{
					// The assignment field exists and is an object
					if (JsonObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
					{
						// Deserialize the assignment object
						Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
					}
					else
					{
						// The assignment field is either null or missing, so handle it as null
						Response.Assignment = FEGIK_AssignmentStruct("null");
					}
				}
				else
				{
					// The assignment field is either null or missing, so handle it as null
					Response.Assignment = FEGIK_AssignmentStruct("null");
				}
				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the Matchmaker is down or the URL is incorrect or is not released"));
	}
}

void UEGIK_CreateMatchmakingTicket::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL(Var_MatchmakingStruct.MatchmakingURL + "/tickets");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("X-Real-Ip", Var_MatchmakingStruct.RealIp);
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("profile", Var_MatchmakingStruct.Profile);
	TSharedPtr<FJsonObject> AttributesJsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Var_MatchmakingStruct.Attributes);
	if (FJsonSerializer::Deserialize(Reader, AttributesJsonObject) && AttributesJsonObject.IsValid())
	{
		JsonObject->SetObjectField(TEXT("attributes"), AttributesJsonObject);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse attributes JSON string."));
	}
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(JsonString);
	UE_LOG(LogTemp, Warning, TEXT("Request: %s"), *JsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateMatchmakingTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
