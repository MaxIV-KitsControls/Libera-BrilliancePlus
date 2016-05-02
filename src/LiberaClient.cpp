/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaClient.cpp 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#include <istd/trace.h>

// MCI includes
#include <mci/node.h>
#include <mci/mci.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include "LiberaBrilliancePlus.h"

#include "LiberaClient.h"

/**
 * Constructor with member initializations.
 */
LiberaClient::LiberaClient(Tango::DeviceImpl *a_deviceServer, std::string ip_address)
  : m_connected(false),
    m_running(false),
	m_errorFlag(false),
    m_thread(),
    m_deviceServer(a_deviceServer)
{
    m_ip_address = "127.0.0.1";
    m_thread = std::thread(std::ref(*this));
    // safety check, wait that thread function has started
    while (!m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
    if (!ip_address.empty())
    {
      m_ip_address = ip_address;
    }
}

/**
 * Wait for update thread to finish and delete destroy created objects.
 */
LiberaClient::~LiberaClient()
{
    istd_FTRC();
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_signals.clear(); // destroy signal objects
    m_attr_pm.clear(); // destroy platform attributes objects
    m_attr.clear(); // destroy atribute objects
}

/**
 * Call notifier function.
 */
void LiberaClient::Notify(LiberaAttr *a_attr)
{
    istd_FTRC();
    m_notify[a_attr]();
}

/**
 * Method for updating all attributes on the list. Its periodically called from
 * the update thread.
 */
void LiberaClient::UpdateAttr()
{
    istd_FTRC();
    try {
        for (auto i = m_attr.begin(); i != m_attr.end(); ++i) {
            (*i)->Read(m_root);
        }
        for (auto i = m_attr_pm.begin(); i != m_attr_pm.end(); ++i) {
            (*i)->Read(m_platform);
        }
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while reading from node!");
        istd_TRC(istd::eTrcLow, e.what());
        // let the server know it
        //m_deviceServer->set_lib_error(e.what());
        m_errorFlag = true;
        m_errorStatus = e.what();
        m_connected = false;
    }
}
/**
 * Periodically read all attribute values.
 */
void LiberaClient::operator()()
{
    istd_FTRC();
    // thread function has started
    m_running = true;
    while (m_running) {
        if (m_connected) {
            UpdateAttr();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        else {
            // wait for stop running
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    istd_TRC(istd::eTrcHigh, "Exit attribute update thread");
}

/**
 * Call execute on the given ireg node.
 */
bool LiberaClient::Execute(const std::string &a_path)
{
    istd_FTRC();
    bool res = false;
    try {
        res = m_root.GetNode(mci::Tokenize(a_path)).Execute();
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown on execute node!");
        istd_TRC(istd::eTrcLow, e.what());
    }
    return res;
}

/**
 * Recursively collect values for all sub-nodes.
 */
void LiberaClient::TreeWalk(
    const mci::Node &a_node, Tango::DevVarStringArray *a_out)
{
    std::string s = mci::ToString(a_node.GetRelPath());

    if (a_node.GetValueType() != mci::eNvUndefined && a_node.IsReadable()) {
        s = s + "=" + a_node.ToString(0);
    }
    (*a_out)[a_out->length() - 1] = CORBA::string_dup(s.c_str());

    for (size_t i(0); i < a_node.GetNodeCount(); ++i) {
        a_out->length(a_out->length() + 1);
        TreeWalk(a_node.GetNode(i), a_out);
    }
}

/**
 * Fill the output argument with value of the ireg node and its sub-nodes.
 */
bool LiberaClient::MagicCommand(
    const std::string &a_path, Tango::DevVarStringArray *a_out)
{
    istd_FTRC();
    bool res = false;
    try {
        if (a_path == "dump") {
            TreeWalk(m_root, a_out);
        }
        else {
            TreeWalk(m_root.GetNode(mci::Tokenize(std::string(a_path))), a_out);
        }
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown on read node!");
        istd_TRC(istd::eTrcLow, e.what());
        a_out->length(1);
        (*a_out)[0] = CORBA::string_dup(e.what());
    }
    return res;
}

/**
 * Connection handling method.
 */
void LiberaClient::Connect(mci::Node &a_root, mci::Root a_type)
{
    // Destroy root node to force disconnect
    if (a_root.IsValid()) {
        try {
            a_root.Destroy();
        }
        catch (istd::Exception e)
        {
            istd_TRC(istd::eTrcLow, "Exception thrown while destroying root node!");
            istd_TRC(istd::eTrcLow, e.what());
        }
        a_root = mci::Node();
    }

    // disconnect if connected
    try {
        mci::Disconnect(m_ip_address.c_str(), a_type);
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while disconnecting root node!");
        istd_TRC(istd::eTrcLow, e.what());
    }

    // make new connection
    try {
        a_root = mci::Connect(m_ip_address.c_str(), a_type);
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while connecting root node!");
        istd_TRC(istd::eTrcLow, e.what());
    }
}

/**
 * Connect to application and platform daemons.
 */
bool LiberaClient::Connect()
{
    istd_FTRC();

    m_connected = false;

    Connect(m_root, mci::Root::Application);
    Connect(m_platform, mci::Root::Platform);

    // update attributes for the first time
    if (m_root.IsValid() && m_platform.IsValid()) {
        // set root node connection for signals
        for (auto i = m_signals.begin(); i != m_signals.end(); ++i) {
            if (!(*i)->Connect(m_root)) {
                m_connected = false;
                istd_TRC(istd::eTrcLow, "Connection to signals failed.");
                return false;
            }
        }
        // start attribute update loop
        m_connected = true;
        istd_TRC(istd::eTrcLow, "Connection to platform and application succeeded.");
    }
    else {
        istd_TRC(istd::eTrcLow, "Connection to platform or application failed.");
    }
    return m_connected;
}

void LiberaClient::Disconnect(mci::Node &a_root, mci::Root a_type)
{
    // destroy root node to force disconnect
    try {
        a_root.Destroy();
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while destroying root node!");
        istd_TRC(istd::eTrcLow, e.what());
    }
    a_root = mci::Node();

    // disconnect if connected
    mci::Disconnect(m_ip_address.c_str(), a_type);
}

void LiberaClient::Disconnect()
{
    istd_FTRC();

    // stop attribute update loop
    m_connected = false;

    Disconnect(m_root, mci::Root::Application);
    Disconnect(m_platform, mci::Root::Platform);
}

bool LiberaClient::IsConnected()
{
    return m_connected;
}
