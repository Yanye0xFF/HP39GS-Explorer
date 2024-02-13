#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

void DLL_EXPORT set_size(int img_width, int img_height);
void DLL_EXPORT set_del_tmp(int state);
int DLL_EXPORT create_bitmap(const LPCSTR file_name);
int DLL_EXPORT created_bitmap_packed(const LPCSTR file_name);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
