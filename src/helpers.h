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

HRESULT FieldDescriptorCoAllocCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd
);

HRESULT FieldDescriptorCopy(
    const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR &rcpfd,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR *pcpfd
);

HRESULT UnicodeStringInitWithString(
    PWSTR pwz,
    UNICODE_STRING *pus
);

HRESULT KerbInteractiveUnlockLogonInit(
    PWSTR pwzDomain,
    PWSTR pwzUsername,
    PWSTR pwzPassword,
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    KERB_INTERACTIVE_UNLOCK_LOGON *pkiul
);

HRESULT KerbInteractiveUnlockLogonPack(
    const KERB_INTERACTIVE_UNLOCK_LOGON &rkiulIn,
    BYTE **prgb,
    DWORD *pcb
);

HRESULT RetrieveNegotiateAuthPackage(
    ULONG *pulAuthPackage
);

HRESULT ProtectIfNecessaryAndCopyPassword(
    PCWSTR pwzPassword,
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    PWSTR *ppwzProtectedPassword
);

HRESULT KerbInteractiveUnlockLogonRepackNative(
    BYTE *rgbWow,
    DWORD cbWow,
    BYTE **prgbNative,
    DWORD *pcbNative
);

void KerbInteractiveUnlockLogonUnpackInPlace(
    KERB_INTERACTIVE_UNLOCK_LOGON *pkiul,
    DWORD cb
);

HRESULT DomainUsernameStringAlloc(
    PCWSTR pwszDomain,
    PCWSTR pwszUsername,
    PWSTR *ppwszDomainUsername
);

HRESULT SplitDomainAndUsername(PCWSTR pszQualifiedUserName, PWSTR *ppszDomain,
                               PWSTR *ppszUsername);
