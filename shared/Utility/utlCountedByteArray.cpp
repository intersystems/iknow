//**********************************************************************
//* Copyright (c) 2000 InterSystems, Corp.
//* Cambridge, Massachusetts, U.S.A.  All rights reserved.
//* Confidential, unpublished property of InterSystems.
//**********************************************************************
//* utlCountedByteArray.cpp: Implementation of Counted Binary Array
//**********************************************************************
//* Change History:
//* Dave McCaldon:  26/1/2000   Initial Implementation.
//**********************************************************************

#ifndef _utlCountedByteArray_h_
#include "utlCountedByteArray.h"
#endif

#ifndef _sysCom_h_
#include "sysCom.h"
#endif


/**
 * A handy function for dumping a byte array out to an STL string
 *
 * @param p_pbData    Pointer to the byte array to dump
 * @param p_cbData    Size of the byte array
 * @param p_rsOutput  STL String output buffer (to be appended to)
 */
void DumpByteArray(const BYTE* p_pbData, size_t p_cbData, String& p_rsOutput)
{
    const size_t t_cchWidth  = 16;
    const size_t t_cchOffset = 4;
    size_t       t_cBytes;
    size_t       t_cOffset;
    size_t       t_i;
    size_t       t_j;
    Char      t_szWork[256];
    size_t       t_iWork;

    static    Char t_rgHexChars[] =
    {
        COM_CHAR('0'), COM_CHAR('1'), COM_CHAR('2'), COM_CHAR('3'),
        COM_CHAR('4'), COM_CHAR('5'), COM_CHAR('6'), COM_CHAR('7'),
        COM_CHAR('8'), COM_CHAR('9'), COM_CHAR('A'), COM_CHAR('B'),
        COM_CHAR('C'), COM_CHAR('D'), COM_CHAR('E'), COM_CHAR('F')
    };

    p_rsOutput.resize(0);

    for (t_cOffset = 0; t_cOffset < p_cbData; )
    {
        // Compute the number of bytes to output in this line
        t_cBytes = p_cbData - t_cOffset;
        if (t_cBytes > t_cchWidth) t_cBytes = t_cchWidth;

        t_iWork = 0;

        // Format the offset
        t_j = t_cOffset;
        for (t_i = 0; t_i < t_cchOffset; t_i++)
        {
            t_szWork[t_cchOffset - (t_i + 1)] = t_rgHexChars[t_j & 0xF];
            t_j >>= 4;
            t_iWork++;
        }

        t_szWork[t_iWork++] = COM_CHAR(':');
        t_szWork[t_iWork++] = COM_CHAR(' ');

        // Dump out the bytes in Hex
        for (t_i = 0; t_i < t_cBytes; t_i++)
        {
            BYTE t_bValue = p_pbData[t_cOffset + t_i];

            t_szWork[t_iWork++] = t_rgHexChars[(t_bValue >> 4) & 0xF];
            t_szWork[t_iWork++] = t_rgHexChars[t_bValue & 0xF];
            t_szWork[t_iWork++] = (t_i == (t_cchWidth / 2)-1) ? COM_CHAR(':')
                                                              : COM_CHAR(' ');
        }

        // Pad any remainder (up to the width) with "**"
        for (t_i = t_cBytes; t_i < t_cchWidth; t_i++)
        {
            t_szWork[t_iWork++] = COM_CHAR('*');
            t_szWork[t_iWork++] = COM_CHAR('*');
            t_szWork[t_iWork++] = COM_CHAR(' ');
        }

        // Spacer before we start to dump the ASCII data
        t_szWork[t_iWork++] = COM_CHAR(' ');
        t_szWork[t_iWork++] = COM_CHAR(':');
        t_szWork[t_iWork++] = COM_CHAR(' ');

        // Dump any printable ASCII chars
        for (t_i = 0; t_i < t_cBytes; t_i++)
        {
            Char t_c = static_cast<Char>(p_pbData[t_cOffset + t_i]);
            t_szWork[t_iWork++] = isprint(t_c) ? t_c : COM_CHAR('?');
        }

        // End the line ...
#ifdef _WIN32
        t_szWork[t_iWork++] = COM_CHAR('\r');
#endif
        t_szWork[t_iWork++] = COM_CHAR('\n');
        t_szWork[t_iWork++] = 0;

        // Append to the output buffer and move on
        p_rsOutput.append(t_szWork);
        t_cOffset += t_cBytes;
    }
}


#ifdef M_DEBUG
#include "utlCountedByteArray.inl"
#endif
