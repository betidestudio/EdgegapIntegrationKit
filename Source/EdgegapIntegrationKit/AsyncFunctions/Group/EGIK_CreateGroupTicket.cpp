// Copyright (c) Betide Studio. All Rights Reserved.


#include "EGIK_CreateGroupTicket.h"

#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_CreateGroupTicket* UEGIK_CreateGroupTicket::CreateGroupTicket(const FEGIK_CreateGroupTicketRequest& Request)
{
	UEGIK_CreateGroupTicket* Node = NewObject<UEGIK_CreateGroupTicket>();
	Node->Var_Request = Request;
	return Node;
}

void UEGIK_CreateGroupTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
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

void UEGIK_CreateGroupTicket::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetURL(Var_Request.MatchmakerUrl + "/group-tickets");
	Request->SetHeader("Authorization", Var_Request.AuthToken);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("profile_id", Var_Request.ProfileId);

	TArray<TSharedPtr<FJsonValue>> MembersArray;
	for (const FEGIK_MemberTicket& Member : Var_Request.Members)
	{
		TSharedPtr<FJsonObject> MemberObject = MakeShareable(new FJsonObject);
		MemberObject->SetStringField("player_ip", Member.PlayerIp);
		MemberObject->SetStringField("profile", Var_Request.ProfileId);

		TSharedPtr<FJsonObject> AttributesObject = MakeShareable(new FJsonObject);
		for (const FEGIK_KeyValue& Attribute : Member.Attributes)
		{
			AttributesObject->SetStringField(Attribute.Key, Attribute.Value);
		}

		TSharedPtr<FJsonObject> BeaconsObject = MakeShareable(new FJsonObject);
		for (const FEGIK_KeyValue& Beacon : Member.Beacons)
		{
			BeaconsObject->SetNumberField(Beacon.Key, FCString::Atof(*Beacon.Value));
		}
		AttributesObject->SetObjectField("beacons", BeaconsObject);
		MemberObject->SetObjectField("attributes", AttributesObject);

		MembersArray.Add(MakeShareable(new FJsonValueObject(MemberObject)));
	}
	JsonObject->SetArrayField("player_tickets", MembersArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(JsonString);

	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateGroupTicket::OnResponseReceived);
	Request->ProcessRequest();
}
