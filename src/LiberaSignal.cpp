/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.cpp 18420 2013-01-10 14:26:03Z tomaz.beltram $
 */

#include <chrono>

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include <istd/trace.h>
#include <mci/mci.h>
#include <mci/mci_util.h>
#include <isig/signal_source.h>

#include "LiberaSignal.h"

LiberaSignal::LiberaSignal(const std::string &a_path, size_t a_length,
    Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize)
  : m_running(false),
    m_thread(),
    m_period(2000),
    m_enabled(a_enabled),
    m_length(a_bufSize),
    m_connected(false),
    m_mode(isig::eModeDodNow),
    m_path(a_path)
{
    istd_FTRC();
    m_enabled = new Tango::DevBoolean;
    *m_enabled = false;

    m_length = new Tango::DevLong;
    *m_length = a_length;

    m_thread = std::thread(std::ref(*this));
    // safety check, wait that thread function has started
    while (!m_running) {
        ::usleep(100000);
    };
}

LiberaSignal::~LiberaSignal()
{
    istd_FTRC();
    Stop();
    delete m_enabled;
    delete m_length;
    istd_TRC(istd::eTrcDetail, "Destroyed base signal for: " << m_path);
}

/**
 * Stop method must be called from derived class destructor before deleting
 * its data members in order to prevent further data modifications.
 */
void LiberaSignal::Stop()
{
    istd_FTRC();
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/**
 * This is internal thread for continuous acquisition. The UpdateSignal method
 * is implemented in derived class.
 */
void LiberaSignal::operator()()
{
    istd_FTRC();
    // thread function has started
    m_running = true;
    while (m_running) {
        if (*m_enabled && m_connected) {
            istd_TRC(istd::eTrcDetail, "Update from thread for: " << m_path);
            Update();
            if (m_mode == isig::eModeDodNow) {
                // In order to avoid busy loop the dod acquisition with
                // eModeDodNow waits here, since Read() is immediate.
                ::usleep(m_period * 1000);
            }
        }
        else {
            // wait for stop running
            ::usleep(100000);
        }
    }
    istd_TRC(istd::eTrcHigh, "Exit update thread for: " << m_path);
}

/**
 * Public method for data acquisition called either from the internal thread or
 * directly to read signal data. It will disable signal and disconnect in case
 * of error.
 */
void LiberaSignal::Update()
{
    istd_FTRC();

    try {
        UpdateSignal();
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while reading signal: " << m_path);
        istd_TRC(istd::eTrcLow, e.what());
        Disable();
        m_connected = false;
    }
}

/**
 * Method for establishing connection to the signal using its ireg node.
 */
bool LiberaSignal::Connect(mci::Node &a_root)
{
    istd_FTRC();
    m_connected = false;
    m_root = a_root;
    try {
        mci::Node sNode = m_root.GetNode(mci::Tokenize(m_path));
        Initialize(sNode);
        m_connected = true;
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while connecting signal: " << m_path);
        istd_TRC(istd::eTrcLow, e.what());
    }
    return m_connected;
}

void LiberaSignal::SetMode(isig::AccessMode_e  a_mode)
{
    m_mode = a_mode;
}

isig::AccessMode_e LiberaSignal::GetMode()
{
    return m_mode;
}

void LiberaSignal::Enable()
{
    *m_enabled = true;
}

void LiberaSignal::Disable()
{
    *m_enabled = false;
}

void LiberaSignal::SetPeriod(uint32_t a_period)
{
    m_period = a_period;
}

size_t LiberaSignal::GetLength()
{
    return *m_length;
}

void LiberaSignal::SetLength(size_t a_length)
{
    *m_length = a_length;
}
