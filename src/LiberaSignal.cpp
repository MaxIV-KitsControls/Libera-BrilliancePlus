/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.cpp 18368 2012-12-18 14:55:03Z tomaz.beltram $
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
    m_offset(0),
    m_enabled(a_enabled),
    m_length(a_bufSize),
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
}

LiberaSignal::~LiberaSignal()
{
    istd_FTRC();
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    delete m_enabled;
    delete m_length;
    istd_TRC(istd::eTrcDetail, "Destroyed base signal for: " << m_path);
}

void LiberaSignal::operator()()
{
    istd_FTRC();
    // thread function has started
    m_running = true;
    while (m_running) {
        if (*m_enabled) {
            Update();
            std::this_thread::sleep_for(std::chrono::milliseconds(m_period));
        }
        else {
            // wait for stop running
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    istd_TRC(istd::eTrcHigh, "Exit update thread for: " << m_path);
}

bool LiberaSignal::Connect(mci::Node &a_root)
{
    istd_FTRC();
    bool res(false);
    m_root = a_root;
    try {
        mci::Node sNode = m_root.GetNode(mci::Tokenize(m_path));
        //isig::SignalSourceSharedPtr signal = mci::CreateRemoteSignal(sNode);
        Initialize(sNode);
        res = true;
    }
    catch (istd::Exception e)
    {
        istd_TRC(istd::eTrcLow, "Exception thrown while connecting signal: " << m_path);
        istd_TRC(istd::eTrcLow, e.what());
    }
    return res;
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

void LiberaSignal::SetOffset(int32_t a_offset)
{
    m_offset = a_offset;
}

int32_t LiberaSignal::GetOffset()
{
    return m_offset;
}

size_t LiberaSignal::GetLength()
{
    return *m_length;
}

void LiberaSignal::SetLength(size_t a_length)
{
    *m_length = a_length;
}

/**
 * Method for differentiating between stream and data on demand access type.
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
    }
}
