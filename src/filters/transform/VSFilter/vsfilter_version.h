#ifndef ISPP_INVOKED
/*
 * (C) 2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#endif // ISPP_INVOKED

#ifndef VSFILTER_VERSION_H
#define VSFILTER_VERSION_H

#include "version.h"

#define VSFILTER_VERSION_MAJOR   2
#define VSFILTER_VERSION_MINOR   41

#define VSFILTER_COPYRIGHT_STR  _T("\nCopyright © 2001-2013 MPC-HC Team")

#ifndef ISPP_INVOKED

#define VSFILTER_VERSION_STR    MAKE_STR(VSFILTER_VERSION_MAJOR) _T(".") \
                                MAKE_STR(VSFILTER_VERSION_MINOR) _T(".") \
                                MAKE_STR(MPC_VERSION_REV)

#endif // ISPP_INVOKED

#ifdef _WIN64
#define VSFILTER_VERSION        _T("VSFilter ") VSFILTER_VERSION_STR \
                                _T(" (") MPC_VERSION_ARCH _T(")") VSFILTER_COPYRIGHT_STR
#else
#define VSFILTER_VERSION        _T("VSFilter ") VSFILTER_VERSION_STR \
                                VSFILTER_COPYRIGHT_STR
#endif

#endif // VSFILTER_VERSION_H
