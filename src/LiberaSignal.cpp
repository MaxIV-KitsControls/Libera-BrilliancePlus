/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.cpp 18273 2012-11-30 15:28:11Z tomaz.beltram $
 */

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include <mci/mci.h>
#include <mci/mci_util.h>

#include "LiberaSignal.h"

LiberaSignalBase::LiberaSignalBase(const char *a_path, size_t a_length,
    Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize)
  : m_running(false),
    m_thread(),
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

LiberaSignalBase::~LiberaSignalBase()
{
    istd_FTRC();
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    delete m_enabled;
    delete m_length;
}

void LiberaSignalBase::operator()()
{
    istd_FTRC();
    // thread function has started
    m_running = true;
    while (m_running) {
        if (*m_enabled) {
            try {
                Update();
            }
            catch (istd::Exception e)
            {
                istd_TRC(istd::eTrcLow, "Exception thrown while reading signal!");
                istd_TRC(istd::eTrcLow, e.what());
                Disable();
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        else {
            // wait for stop running
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void LiberaSignalBase::Enable(mci::Node &a_root)
{
    m_root = a_root;
    *m_enabled = true;
}

void LiberaSignalBase::Disable()
{
    *m_enabled = false;
}

size_t LiberaSignalBase::GetLength()
{
    return *m_length;
}

void LiberaSignalBase::SetLength(size_t a_length)
{
    *m_length = a_length;
}

/**
 * Method for differentiating between stream and data on demand access type.
 */
void LiberaSignalBase::Update()
{
    istd_FTRC();

    mci::Node sNode = m_root.GetNode(mci::Tokenize(m_path));
    auto rSignal = mci::CreateRemoteSignal(sNode);

    if (rSignal->AccessType() == isig::eAccessStream) {
        UpdateStream(rSignal);
    }
    else if (rSignal->AccessType() == isig::eAccessDataOnDemand) {
        UpdateDod(rSignal);
    }
    else {
        istd_TRC(istd::eTrcLow, "Unsupported signal access mode.")
    }
}
