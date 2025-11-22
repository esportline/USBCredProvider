//
// CUSBCredential - Représente une tentative de connexion avec authentification USB
//

#pragma once

#include "helpers.h"
#include "common.h"
#include "dll.h"
#include <credentialprovider.h>

class CUSBCredential : public ICredentialProviderCredential {
public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef() {
        return ++_cRef;
    }

    IFACEMETHODIMP_(ULONG) Release() {
        LONG cRef = --_cRef;
        if (!cRef) {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);

    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();

    IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
                                  CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
                                  CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz);
    IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel);
    IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);
    IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem);
    IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem);

    IFACEMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
    IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
    IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
    IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
                                     CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
                                     PWSTR* ppwszOptionalStatusText,
                                     CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus,
                                 NTSTATUS ntsSubstatus,
                                 PWSTR* ppwszOptionalStatusText,
                                 CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                        const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                        const FIELD_STATE_PAIR* rgfsp,
                        PCWSTR pwzUsername,
                        PCWSTR pwzDomain);
    CUSBCredential();

    virtual ~CUSBCredential();

private:
    // Vérifie si une clé USB autorisée est présente
    HRESULT CheckUSBPresence();

    // Récupère le nom d'utilisateur actuel de Windows
    HRESULT GetCurrentUsername(PWSTR* ppwszUsername);

private:
    LONG _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus; // Le scénario d'utilisation pour lequel nous avons été énumérés
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgCredProvFieldDescriptors[SFI_NUM_FIELDS]; // Un tableau contenant le type et le nom de chaque champ
    FIELD_STATE_PAIR _rgFieldStatePairs[SFI_NUM_FIELDS]; // Un tableau indiquant quels champs sont actifs et interactifs
    PWSTR _rgFieldStrings[SFI_NUM_FIELDS]; // Un tableau contenant les valeurs de chaîne des champs
    ICredentialProviderCredentialEvents* _pCredProvCredentialEvents; // Pointeur vers notre UI

    PWSTR _pwzUsername; // Nom d'utilisateur
    PWSTR _pwzDomain; // Domaine
    BOOL _fChecked; // Checkbox
    DWORD _dwComboIndex; // Combobox

    PWSTR _pwzQualifiedUserName; // Domaine\nom d'utilisateur
    bool _fIsLocalUser; // Si l'utilisateur est local ou du domaine
};
