//
// Created by enimaloc on 22/11/2025.
//

#pragma once

extern HINSTANCE g_hinst;
#define HINST_THISDLL g_hinst

void DllAddRef();
void DllRelease();