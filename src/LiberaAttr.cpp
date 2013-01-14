/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaAttr.cpp 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#include "LiberaAttr.h"
#include "LiberaClient.h"

/**
 * Call the client with this attribute pointer.
 */
void LiberaAttr::Notify()
{
    if (m_client) {
        m_client->Notify(this);
    }
}
