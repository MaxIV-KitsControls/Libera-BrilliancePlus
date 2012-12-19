/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaAttr.cpp 18339 2012-12-14 12:09:03Z tomaz.beltram $
 */

#include "LiberaAttr.h"
#include "LiberaClient.h"

void LiberaAttr::Notify()
{
    if (m_client) {
        m_client->Notify(this);
    }
}
