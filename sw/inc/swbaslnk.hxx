/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_SWBASLNK_HXX
#define INCLUDED_SW_INC_SWBASLNK_HXX

#include <sfx2/lnkbase.hxx>

class SwNode;
class SwCntntNode;
class ReReadThread;
long GrfNodeChanged( void* pLink, void* pCaller );

class SwBaseLink : public ::sfx2::SvBaseLink
{
    friend long GrfNodeChanged( void* pLink, void* pCaller );

    SwCntntNode* pCntntNode;
    bool bSwapIn : 1;
    bool bNoDataFlag : 1;
    bool bIgnoreDataChanged : 1;
    ReReadThread* m_pReReadThread;

protected:
    SwBaseLink(): m_pReReadThread(0) {}

public:
    TYPEINFO_OVERRIDE();

    SwBaseLink( SfxLinkUpdateMode nMode, SotClipboardFormatId nFormat, SwCntntNode* pNode = 0 )
        : ::sfx2::SvBaseLink( nMode, nFormat ), pCntntNode( pNode ),
        bSwapIn( false ), bNoDataFlag( false ), bIgnoreDataChanged( false ),
        m_pReReadThread(0)
    {}
    virtual ~SwBaseLink();

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue ) SAL_OVERRIDE;

    virtual void Closed() SAL_OVERRIDE;

    virtual const SwNode* GetAnchor() const;

    SwCntntNode *GetCntntNode() { return pCntntNode; }

    // For graphics only.
    bool SwapIn( bool bWaitForData = false, bool bNativFormat = false );

    bool Connect() { return 0 != SvBaseLink::GetRealObject(); }

    // Only for graphics-links (for switching between DDE / Grf-link).
    void SetObjType( sal_uInt16 nType ) { SvBaseLink::SetObjType( nType ); }

    bool IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, sal_Int32 nStt = 0,
                            sal_Int32 nEnd = -1 ) const;

    void SetNoDataFlag() { bNoDataFlag = true; }
    bool ChkNoDataFlag() { const bool bRet = bNoDataFlag; bNoDataFlag = false; return bRet; }
    bool IsNoDataFlag() const { return bNoDataFlag; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
