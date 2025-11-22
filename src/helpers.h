//
// Created by enimaloc on 22/11/2025.
//

#pragma once

#pragma warning(push)
#pragma warning(disable : 28251)
#include <credentialprovider.h>
#include <ntsecapi.h>
#pragma warning(pop)

#define SECURITY_WIN32
#include <security.h>
#include <intsafe.h>

#include <windows.h>
#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable : 4995)
#include <shlwapi.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 28301)
#include <wincred.h>
#pragma warning(pop)

// MinGW compatibility: Define missing constants AFTER Windows headers
#ifndef CPFG_CREDENTIAL_PROVIDER_LOGO
#define CPFG_CREDENTIAL_PROVIDER_LOGO 0
#endif

#ifndef CPFG_CREDENTIAL_PROVIDER_LABEL
#define CPFG_CREDENTIAL_PROVIDER_LABEL 1
#endif

#ifndef UNLEN
#define UNLEN 256
#endif

// MinGW compatibility: Map old-style SAL annotations to modern ones
// These are defined AFTER all system headers to avoid conflicts with STL
#ifndef __in
#define __in _In_
#define __in_opt _In_opt_
#define __out _Out_
#define __out_opt _Out_opt_
#define __inout _Inout_
#define __inout_opt _Inout_opt_
#define __deref_out _Outptr_
#define __deref_out_opt _Outptr_opt_
#define __in_ecount(x) _In_reads_(x)
#define __out_ecount(x) _Out_writes_(x)
#define __deref_out_range(x,y)
#define __in_bcount(x) _In_reads_bytes_(x)
#define __out_bcount(x) _Out_writes_bytes_(x)
#define __in_range(x,y)
#define __in_reads(x) _In_reads_(x)
#define __in_reads_opt(x) _In_reads_opt_(x)
#define __in_reads_bytes(x) _In_reads_bytes_(x)
#define __in_reads_bytes_opt(x) _In_reads_bytes_opt_(x)
#define __out_writes(x) _Out_writes_(x)
#define __out_writes_opt(x) _Out_writes_opt_(x)
#define __out_writes_bytes(x) _Out_writes_bytes_(x)
#define __out_writes_bytes_opt(x) _Out_writes_bytes_opt_(x)
#define __deref_in_range(x,y)
#define __field_ecount(x)
#define __field_bcount(x)
#define __override override
#endif

HRESULT FieldDescriptorCoAllocCopy(
    _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd
);

HRESULT FieldDescriptorCopy(
    _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    _Out_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR *pcpfd
);

HRESULT UnicodeStringInitWithString(
    _In_ PWSTR pwz,
    _Out_ UNICODE_STRING *pus
);

HRESULT KerbInteractiveUnlockLogonInit(
    _In_ PWSTR pwzDomain,
    _In_ PWSTR pwzUsername,
    _In_ PWSTR pwzPassword,
    _In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    _Out_ KERB_INTERACTIVE_UNLOCK_LOGON *pkiul
);

HRESULT KerbInteractiveUnlockLogonPack(
    _In_ const KERB_INTERACTIVE_UNLOCK_LOGON &rkiulIn,
    _Outptr_result_bytebuffer_(*pcb) BYTE **prgb,
    _Out_ DWORD *pcb
);

HRESULT RetrieveNegotiateAuthPackage(
    _Out_ ULONG *pulAuthPackage
);

HRESULT ProtectIfNecessaryAndCopyPassword(
    _In_ PCWSTR pwzPassword,
    _In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    _Outptr_result_nullonfailure_ PWSTR *ppwzProtectedPassword
);

HRESULT KerbInteractiveUnlockLogonRepackNative(
    _In_reads_bytes_(cbWow) BYTE *rgbWow,
    _In_ DWORD cbWow,
    _Outptr_result_bytebuffer_(*pcbNative) BYTE **prgbNative,
    _Out_ DWORD *pcbNative
);

void KerbInteractiveUnlockLogonUnpackInPlace(
    _Inout_updates_bytes_(cb) KERB_INTERACTIVE_UNLOCK_LOGON *pkiul,
    DWORD cb
);

HRESULT DomainUsernameStringAlloc(
    _In_ PCWSTR pwszDomain,
    _In_ PCWSTR pwszUsername,
    _Outptr_result_nullonfailure_ PWSTR *ppwszDomainUsername
);

HRESULT SplitDomainAndUsername(_In_ PCWSTR pszQualifiedUserName, _Outptr_result_nullonfailure_ PWSTR *ppszDomain,
                               _Outptr_result_nullonfailure_ PWSTR *ppszUsername);
