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
#ifndef INCLUDED_SFX2_PASSWD_HXX
#define INCLUDED_SFX2_PASSWD_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <sfx2/app.hxx>
#include <o3tl/typed_flags_set.hxx>

// defines ---------------------------------------------------------------

enum class SfxShowExtras
{
    NONE      = 0x0000,
    USER      = 0x0001,
    CONFIRM   = 0x0002,
    PASSWORD2 = 0x0004,
    CONFIRM2  = 0x0008,
    ALL       = USER | CONFIRM
};
namespace o3tl
{
    template<> struct typed_flags<SfxShowExtras> : is_typed_flags<SfxShowExtras, 0x0f> {};
}

// class SfxPasswordDialog -----------------------------------------------

class SFX2_DLLPUBLIC SfxPasswordDialog : public ModalDialog
{
private:
    VclFrame*       mpPassword1Box;
    FixedText*      mpUserFT;
    Edit*           mpUserED;
    FixedText*      mpPassword1FT;
    Edit*           mpPassword1ED;
    FixedText*      mpConfirm1FT;
    Edit*           mpConfirm1ED;

    VclFrame*       mpPassword2Box;
    FixedText*      mpPassword2FT;
    Edit*           mpPassword2ED;
    FixedText*      mpConfirm2FT;
    Edit*           mpConfirm2ED;

    FixedText*      mpMinLengthFT;

    OKButton*       mpOKBtn;

    OUString        maMinLenPwdStr;
    OUString        maMinLenPwdStr1;
    OUString        maEmptyPwdStr;
    OUString        maMainPwdStr;
    sal_uInt16      mnMinLen;
    SfxShowExtras  mnExtras;

    bool            mbAsciiOnly;
    DECL_DLLPRIVATE_LINK(EditModifyHdl, Edit*);
    DECL_DLLPRIVATE_LINK(OKHdl, void *);

    void            SetPasswdText();

public:
    SfxPasswordDialog(vcl::Window* pParent, const OUString* pGroupText = NULL);

    OUString GetUser() const
    {
        return mpUserED->GetText();
    }
    OUString GetPassword() const
    {
        return mpPassword1ED->GetText();
    }
    OUString GetConfirm() const
    {
        return mpConfirm1ED->GetText();
    }
    OUString GetPassword2() const
    {
        return mpPassword2ED->GetText();
    }
    OUString GetConfirm2() const
    {
        return mpConfirm2ED->GetText();
    }
    void SetGroup2Text(const OUString& i_rText)
    {
        mpPassword2Box->set_label(i_rText);
    }
    void SetMinLen(sal_uInt16 Len);
    void SetEditHelpId(const OString& rId)
    {
        mpPassword1ED->SetHelpId( rId );
    }
    void ShowExtras(SfxShowExtras nExtras)
    {
        mnExtras = nExtras;
    }
    void AllowAsciiOnly(bool i_bAsciiOnly = true)
    {
        mbAsciiOnly = i_bAsciiOnly;
    }

    void ShowMinLengthText(bool bShow);

    virtual short Execute() SAL_OVERRIDE;
};

#endif // INCLUDED_SFX2_PASSWD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
