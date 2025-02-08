// Copyright (c) Betide Studio. All Rights Reserved.


#include "EGIK_CreateGroupTicket.h"

#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_CreateGroupTicket* UEGIK_CreateGroupTicket::CreateGroupTicket(const FEGIK_CreateGroupTicketRequest& Request)
{
	UEGIK_CreateGroupTicket* Node = NewObject<UEGIK_CreateGroupTicket>();
	Node->Var_Request = Request;
	return Node;
}

void UEGIK_CreateGroupTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	TArray<FEGIK_MatchmakingResponse> ResponseArray;
	if(HttpResponse.IsValid())
	{
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				if(JsonObject->HasField(TEXT("player_tickets")))
				{
					const TArray<TSharedPtr<FJsonValue>> TicketsArray = JsonObject->GetArrayField(TEXT("player_tickets"));
					for (const TSharedPtr<FJsonValue>& TicketValue : TicketsArray)
					{
						TSharedPtr<FJsonObject> TicketObject = TicketValue->AsObject();
						if (TicketObject.IsValid())
						{
							FEGIK_MatchmakingResponse Response;
							if(TicketObject->HasField(TEXT("id")))
							{
								Response.TicketId = TicketObject->GetStringField(TEXT("id"));
							}
							if(TicketObject->HasField(TEXT("status")))
							{
								Response.Status = TicketObject->GetStringField(TEXT("status"));
							}
							if(TicketObject->HasField(TEXT("assignment")))
							{
								Response.Assignment = TicketObject->GetStringField(TEXT("assignment"));
							}
							if(TicketObject->HasField(TEXT("player_ip")))
							{
								Response.IP = TicketObject->GetStringField(TEXT("player_ip"));
							}
							if(TicketObject->HasField(TEXT("profile")))
							{
								Response.GameProfile = TicketObject->GetStringField(TEXT("profile"));
							}
							if(TicketObject->HasField(TEXT("group_id")))
							{
								Response.GroupId = TicketObject->GetStringField(TEXT("group_id"));
							}
							if(TicketObject->HasField(TEXT("created_at")))
							{
								FDateTime CreatedAt;
								FDateTime::Parse(TicketObject->GetStringField(TEXT("created_at")), CreatedAt);
								Response.CreatedAt = CreatedAt;
							}
							ResponseArray.Add(Response);
						}
					}
				}
				OnSuccess.Broadcast(ResponseArray, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
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
		if(!Member.PlayerIp.IsEmpty())
		{
			MemberObject->SetStringField("player_ip", Member.PlayerIp);
		}
		MemberObject->SetStringField("profile", Var_Request.ProfileId);
		TSharedPtr<FJsonObject> AttributesJsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Member.Attributes);
		if (FJsonSerializer::Deserialize(Reader, AttributesJsonObject) && AttributesJsonObject.IsValid())
		{
			MemberObject->SetObjectField(TEXT("attributes"), AttributesJsonObject);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse attributes JSON string."));
		}
		MembersArray.Add(MakeShareable(new FJsonValueObject(MemberObject)));
	}
	JsonObject->SetArrayField("player_tickets", MembersArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(JsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateGroupTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
