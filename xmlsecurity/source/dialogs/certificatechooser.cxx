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


#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>

#include "resourcemanager.hxx"
#include <vcl/msgbox.hxx>
#include <svtools/treelistentry.hxx>

using namespace ::com::sun::star;

#define INVAL_SEL       0xFFFF

sal_uInt16 CertificateChooser::GetSelectedEntryPos() const
{
    sal_uInt16  nSel = INVAL_SEL;

    SvTreeListEntry* pSel = m_pCertLB->FirstSelected();
    if( pSel )
        nSel = (sal_uInt16) reinterpret_cast<sal_uIntPtr>( pSel->GetUserData() );

    return (sal_uInt16) nSel;
}

CertificateChooser::CertificateChooser( vcl::Window* _pParent, uno::Reference< uno::XComponentContext>& _rxCtx, uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment, const SignatureInformations& _rCertsToIgnore )
    : ModalDialog(_pParent, "SelectCertificateDialog", "xmlsec/ui/selectcertificatedialog.ui")
    , maCertsToIgnore( _rCertsToIgnore )
{
    get(m_pOKBtn, "ok");
    get(m_pViewBtn, "viewcert");

    Size aControlSize(275, 122);
    const long nControlWidth = aControlSize.Width();
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    SvSimpleTableContainer *pSignatures = get<SvSimpleTableContainer>("signatures");
    pSignatures->set_width_request(aControlSize.Width());
    pSignatures->set_height_request(aControlSize.Height());

    m_pCertLB = new SvSimpleTable(*pSignatures);
    static long nTabs[] = { 3, 0, 30*nControlWidth/100, 60*nControlWidth/100 };
    m_pCertLB->SetTabs( &nTabs[0] );
    m_pCertLB->InsertHeaderEntry(get<FixedText>("issuedto")->GetText() + "\t" + get<FixedText>("issuedby")->GetText()
        + "\t" + get<FixedText>("expiration")->GetText());
    m_pCertLB->SetSelectHdl( LINK( this, CertificateChooser, CertificateHighlightHdl ) );
    m_pCertLB->SetDoubleClickHdl( LINK( this, CertificateChooser, CertificateSelectHdl ) );
    m_pViewBtn->SetClickHdl( LINK( this, CertificateChooser, ViewButtonHdl ) );

    mxCtx = _rxCtx;
    mxSecurityEnvironment = _rxSecurityEnvironment;
    mbInitialized = false;

    // disable buttons
    CertificateHighlightHdl( NULL );
}

CertificateChooser::~CertificateChooser()
{
    delete m_pCertLB;
}

short CertificateChooser::Execute()
{
    // #i48432#
    // We can't check for personal certificates before raising this dialog,
    // because the mozilla implementation throws a NoPassword exception,
    // if the user pressed cancel, and also if the database does not exist!
    // But in the later case, the is no password query, and the user is confused
    // that nothing happens when pressing "Add..." in the SignatureDialog.

    // PostUserEvent( LINK( this, CertificateChooser, Initialize ) );

    // PostUserLink behavior is to slow, so do it directly before Execute().
    // Problem: This Dialog should be visible right now, and the parent should not be accessible.
    // Show, Update, DIsableInput...

    vcl::Window* pMe = this;
    vcl::Window* pParent = GetParent();
    if ( pParent )
        pParent->EnableInput( false );
    pMe->Show();
    pMe->Update();
    ImplInitialize();
    if ( pParent )
        pParent->EnableInput( true );
    return ModalDialog::Execute();
}

// IMPL_LINK_NOARG(CertificateChooser, Initialize)
void CertificateChooser::ImplInitialize()
{
    if ( !mbInitialized )
    {
        try
        {
            maCerts = mxSecurityEnvironment->getPersonalCertificates();
        }
        catch (security::NoPasswordException&)
        {
        }

        uno::Reference< css::security::XSerialNumberAdapter> xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

        sal_Int32 nCertificates = maCerts.getLength();
        sal_Int32 nCertificatesToIgnore = maCertsToIgnore.size();
        for( sal_Int32 nCert = nCertificates; nCert; )
        {
            uno::Reference< security::XCertificate > xCert = maCerts[ --nCert ];
            bool bIgnoreThis = false;

            // Do we already use that?
            if( nCertificatesToIgnore )
            {
                OUString aIssuerName = xCert->getIssuerName();
                for( sal_Int32 nSig = 0; nSig < nCertificatesToIgnore; ++nSig )
                {
                    const SignatureInformation& rInf = maCertsToIgnore[ nSig ];
                    if ( ( aIssuerName == rInf.ouX509IssuerName ) &&
                        ( xSerialNumberAdapter->toString( xCert->getSerialNumber() ) == rInf.ouX509SerialNumber ) )
                    {
                        bIgnoreThis = true;
                        break;
                    }
                }
            }

            if ( !bIgnoreThis )
            {
                // Check if we have a private key for this...
                long nCertificateCharacters = mxSecurityEnvironment->getCertificateCharacters( xCert );

                if ( !( nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY ) )
                    bIgnoreThis = true;

            }

            if ( bIgnoreThis )
            {
                ::comphelper::removeElementAt( maCerts, nCert );
                nCertificates = maCerts.getLength();
            }
        }

        // fill list of certificates; the first entry will be selected
        for ( sal_Int32 nC = 0; nC < nCertificates; ++nC )
        {
            SvTreeListEntry* pEntry = m_pCertLB->InsertEntry( XmlSec::GetContentPart( maCerts[ nC ]->getSubjectName() )
                + "\t" + XmlSec::GetContentPart( maCerts[ nC ]->getIssuerName() )
                + "\t" + XmlSec::GetDateString( maCerts[ nC ]->getNotValidAfter() ) );
            pEntry->SetUserData( reinterpret_cast<void*>(nC) ); // missuse user data as index
        }

        // enable/disable buttons
        CertificateHighlightHdl( NULL );
        mbInitialized = true;
    }
}


uno::Reference< css::security::XCertificate > CertificateChooser::GetSelectedCertificate()
{
    uno::Reference< css::security::XCertificate > xCert;
    sal_uInt16  nSelected = GetSelectedEntryPos();
    if ( nSelected < maCerts.getLength() )
        xCert = maCerts[ nSelected ];
    return xCert;
}

IMPL_LINK_NOARG(CertificateChooser, CertificateHighlightHdl)
{
    bool bEnable = GetSelectedCertificate().is();
    m_pViewBtn->Enable( bEnable );
    m_pOKBtn->Enable( bEnable );
    return 0;
}

IMPL_LINK_NOARG(CertificateChooser, CertificateSelectHdl)
{
    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK_NOARG(CertificateChooser, ViewButtonHdl)
{
    ImplShowCertificateDetails();
    return 0;
}

void CertificateChooser::ImplShowCertificateDetails()
{
    uno::Reference< css::security::XCertificate > xCert = GetSelectedCertificate();
    if( xCert.is() )
    {
        CertificateViewer aViewer( this, mxSecurityEnvironment, xCert, true );
        aViewer.Execute();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
