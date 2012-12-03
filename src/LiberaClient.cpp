/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaClient.cpp 18277 2012-12-03 12:02:45Z tomaz.beltram $
 */

#include <istd/trace.h>

// MCI includes
#include <mci/node.h>
#include <mci/mci.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include "LiberaAttr.h"
#include "LiberaSignal.h"
#include "LiberaClient.h"

const char *c_localHost = "127.0.0.1";

/**
 * Constructor with member initializations.
 */
LiberaClient::LiberaClient(Tango::Device_4Impl *a_deviceServer, const std::string &a_board)
  : m_deviceServer(a_deviceServer), m_board("boards." + a_board + ".")
{
}

/**
 * Method for updating all attributes on the list.
 */
void LiberaClient::Update()
{
    istd_FTRC();
    try {
        for (auto i = m_attr.begin(); i != m_attr.end(); ++i) {
            (*i)->Read(m_root);
        }
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while reading from node!");
        istd_TRC(istd::eTrcLow, e.what());
        // let the server know it
        m_deviceServer->set_state(Tango::OFF);
    }
}

void LiberaClient::Enable(LiberaSignalBase *a_signal)
{
    istd_FTRC();
    a_signal->Enable(m_root);
}

void LiberaClient::Disable(LiberaSignalBase *a_signal)
{
    istd_FTRC();
    a_signal->Disable();
}

/**
 * Connection handling methods.
 */
bool LiberaClient::Connect()
{
    istd_FTRC();

    // Destroy root node to force disconnect
    if (m_root.IsValid()) {
        try {
            m_root.Destroy();
        }
        catch (istd::Exception e)
        {
            istd_TRC(istd::eTrcLow, "Exception thrown while destroying root node!");
            istd_TRC(istd::eTrcLow, e.what());
        }
        m_root = mci::Node();
    }

    // disconnect if connected
    try {
        mci::Disconnect(c_localHost, mci::Root::Application, mci::c_defaultPort);
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while disconnecting from application!");
        istd_TRC(istd::eTrcLow, e.what());
    }

    // make new connection
    try {
        m_root = mci::Connect(c_localHost, mci::Root::Application, mci::c_defaultPort);
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while connecting to application!");
        istd_TRC(istd::eTrcLow, e.what());
    }

    // update attributes for the first time
    if (m_root.IsValid()) {
        Update();
        return true;
    }
    return false;
}

void LiberaClient::Disconnect()
{
    istd_FTRC();

    // Destroy root node to force disconnect
    try {
        m_root.Destroy();
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while destroying root node!");
        istd_TRC(istd::eTrcLow, e.what());
    }
    m_root = mci::Node();

    // disconnect if connected
    mci::Disconnect(c_localHost, mci::Root::Application, mci::c_defaultPort);
}
