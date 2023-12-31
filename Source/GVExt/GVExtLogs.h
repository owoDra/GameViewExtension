// Copyright (C) 2023 owoDra

#pragma once

#include "Logging/LogMacros.h"

GVEXT_API DECLARE_LOG_CATEGORY_EXTERN(LogGVE, Log, All);

#if !UE_BUILD_SHIPPING

#define GVELOG(FormattedText, ...) UE_LOG(LogGVE, Log, FormattedText, __VA_ARGS__)

#define GVEENSURE(InExpression) ensure(InExpression)
#define GVEENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GVEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GVECHECK(InExpression) check(InExpression)
#define GVECHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GVELOG(FormattedText, ...)

#define GVEENSURE(InExpression) InExpression
#define GVEENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GVEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GVECHECK(InExpression) InExpression
#define GVECHECK_MSG(InExpression, InFormat, ...) InExpression

#endif