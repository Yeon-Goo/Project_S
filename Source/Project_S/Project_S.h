// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Project_S��� Ŀ���� �α� ī�װ� ����
DECLARE_LOG_CATEGORY_EXTERN(Project_S, Log, All);

// FUNCTION �̸��� LINE�� ���
#define PS_LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
// Verbosity()�� ���ڷ� �޾� Verbosity�� �´� PS_LOG_CALLINFO�� ���
#define PS_LOG_S(Verbosity) UE_LOG(Project_S, Verbosity, TEXT("%s"), *PS_LOG_CALLINFO)
//
#define PS_LOG(Verbosity, Format, ...) UE_LOG(Project_S, Verbosity, TEXT("%s"), *PS_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define PS_CHECK(Expr, ...) { if (!(Expr)) { }}