/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignalSAHistory.cpp 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#include <math.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include "LiberaSignalSAHistory.h"

/**
 * Special class template for SA history and statistics. Its based on
 * the LiberaSignalAttr class for the SA stream. It adds history buffer and
 * statistical attributes like mean, root mean square and peak to peak.
 */

LiberaSignalAttr<LiberaSAHistory>::LiberaSignalAttr(
    const char *a_path,
    const size_t a_length,
    Tango::DevBoolean *&a_enabled,
    Tango::DevLong   *&a_SAStatNumSamples_read,
    Tango::DevDouble *&a_VaSA_read,
    Tango::DevDouble *&a_VbSA_read,
    Tango::DevDouble *&a_VcSA_read,
    Tango::DevDouble *&a_VdSA_read,
    Tango::DevDouble *&a_SumSA_read,
    Tango::DevDouble *&a_QuadSA_read,
    Tango::DevDouble *&a_XPosSA_read,
    Tango::DevDouble *&a_ZPosSA_read,
    Tango::DevDouble *&a_XPosSAHistory_read,
    Tango::DevDouble *&a_ZPosSAHistory_read,
    Tango::DevDouble *&a_SumSAHistory_read,
    Tango::DevDouble *&a_XMeanPosSA_read,
    Tango::DevDouble *&a_ZMeanPosSA_read,
    Tango::DevDouble *&a_XRMSPosSA_read,
    Tango::DevDouble *&a_ZRMSPosSA_read,
    Tango::DevDouble *&a_XPeakPosSA_read,
    Tango::DevDouble *&a_ZPeakPosSA_read,
    Tango::DevDouble *&a_SumMeanSA_read)
  :  LiberaSignalAttr<Tango::DevDouble>(a_path, 1, a_enabled, m_SASize,
          a_VaSA_read,
          a_VbSA_read,
          a_VcSA_read,
          a_VdSA_read,
          a_SumSA_read,
          a_QuadSA_read,
          a_XPosSA_read,
          a_ZPosSA_read),
    m_SAStatNumSamples(a_SAStatNumSamples_read),
    m_SumSA(a_SumSA_read),
    m_XPosSA(a_XPosSA_read),
    m_ZPosSA(a_ZPosSA_read),
    m_XPosSAHistory(a_XPosSAHistory_read),
    m_ZPosSAHistory(a_ZPosSAHistory_read),
    m_SumSAHistory(a_SumSAHistory_read)
{
    istd_FTRC();

    m_SAStatNumSamples  = new Tango::DevLong;
    *m_SAStatNumSamples = a_length;

    istd_TRC(istd::eTrcMed, "Alloc for: " << *m_SAStatNumSamples);

    m_SumSAHistory  = new Tango::DevDouble[*m_SAStatNumSamples];
    m_XPosSAHistory = new Tango::DevDouble[*m_SAStatNumSamples];
    m_ZPosSAHistory = new Tango::DevDouble[*m_SAStatNumSamples];

    std::fill(m_SumSAHistory, m_SumSAHistory + *m_SAStatNumSamples, 0.0);
    std::fill(m_XPosSAHistory, m_XPosSAHistory + *m_SAStatNumSamples, 0.0);
    std::fill(m_ZPosSAHistory, m_ZPosSAHistory + *m_SAStatNumSamples, 0.0);
    m_valid = 0;

    a_XMeanPosSA_read = &m_XMeanPosSA;
    a_ZMeanPosSA_read = &m_ZMeanPosSA;
    a_XRMSPosSA_read  = &m_XRMSPosSA;
    a_ZRMSPosSA_read  = &m_ZRMSPosSA;
    a_XPeakPosSA_read = &m_XPeakPosSA;
    a_ZPeakPosSA_read = &m_ZPeakPosSA;
    a_SumMeanSA_read  = &m_SumMeanSA;
}

LiberaSignalAttr<LiberaSAHistory>::~LiberaSignalAttr()
{
    // Protect race with UpdateSignal call, stop update thread first.
    Stop();
    delete [] m_SumSAHistory;
    delete [] m_XPosSAHistory;
    delete [] m_ZPosSAHistory;

    delete m_SAStatNumSamples;
}

/**
 * Change the size of history buffer used for statistics calculation. Note that
 * the underlying stream has the size of one atom and must not be changed.
 */
void LiberaSignalAttr<LiberaSAHistory>::Realloc(size_t a_length)
{
    istd_FTRC();

    std::lock_guard<std::mutex> l(m_data_x);
    istd_TRC(istd::eTrcMed, "Realloc from: " << *m_SAStatNumSamples << " to: " << a_length);
    delete [] m_SumSAHistory;
    delete [] m_XPosSAHistory;
    delete [] m_ZPosSAHistory;

    *m_SAStatNumSamples = a_length;

    m_SumSAHistory  = new Tango::DevDouble[*m_SAStatNumSamples];
    m_XPosSAHistory = new Tango::DevDouble[*m_SAStatNumSamples];
    m_ZPosSAHistory = new Tango::DevDouble[*m_SAStatNumSamples];

    std::fill(m_SumSAHistory, m_SumSAHistory + *m_SAStatNumSamples, 0.0);
    std::fill(m_XPosSAHistory, m_XPosSAHistory + *m_SAStatNumSamples, 0.0);
    std::fill(m_ZPosSAHistory, m_ZPosSAHistory + *m_SAStatNumSamples, 0.0);
    m_valid = 0;
}

/**
 * Copy accumulated data from the buffer to spectrum attribute memory for user
 * access and empty the buffer.
 */
void LiberaSignalAttr<LiberaSAHistory>::GetData()
{
    istd_FTRC();
    std::lock_guard<std::mutex> l(m_data_x);

    // copy buffer and erase processed data
    istd_TRC(istd::eTrcDetail, "GetData: " << *m_SAStatNumSamples);
    Refill(m_sum, m_SumSAHistory);
    Refill(m_xpos, m_XPosSAHistory);
    Refill(m_zpos, m_ZPosSAHistory);

    // calculate statistics
    Tango::DevDouble temp; // placeholder for not exported attributes
    Statistics(m_SumSAHistory, m_SumMeanSA, temp, temp);
    Statistics(m_XPosSAHistory, m_XMeanPosSA, m_XRMSPosSA, m_XPeakPosSA);
    Statistics(m_ZPosSAHistory, m_ZMeanPosSA, m_ZRMSPosSA, m_ZPeakPosSA);

    // update number of valid samples for statistics calculation
    m_valid += m_sum.size();
    if (m_valid > *m_SAStatNumSamples) {
        m_valid = *m_SAStatNumSamples;
    }

    // erase buffers
    m_sum.erase(m_sum.begin(), m_sum.end());
    m_xpos.erase(m_xpos.begin(), m_xpos.end());
    m_zpos.erase(m_zpos.begin(), m_zpos.end());
}

/**
 * Get one stream sample and append it to data buffer.
 */
void LiberaSignalAttr<LiberaSAHistory>::UpdateSignal()
{
    istd_FTRC();
    istd_TRC(istd::eTrcDetail, "UpdateSignal: " << *m_SAStatNumSamples);

    // get SA stream data and store in buffer
    LiberaSignalAttr<Tango::DevDouble>::UpdateSignal();
    LiberaSignalAttr<Tango::DevDouble>::GetData();

    // convert units nm to mm
    *m_XPosSA = *m_XPosSA * 1e-6;
    *m_ZPosSA = *m_ZPosSA * 1e-6;

    // prevent changes in data buffer from the get data thread
    std::lock_guard<std::mutex> l(m_data_x);

    // Shrink data buffer to the size of spectrum attribute memory.
    if (m_sum.size() >= 2 * (*m_SAStatNumSamples)) {
        m_sum.erase(m_sum.begin(), m_sum.begin() + *m_SAStatNumSamples);
        istd_TRC(istd::eTrcHigh, "Reduced sum buffer size to: " << m_sum.size());
    }
    if (m_xpos.size() >= 2 * (*m_SAStatNumSamples)) {
        m_xpos.erase(m_xpos.begin(), m_xpos.begin() + *m_SAStatNumSamples);
        istd_TRC(istd::eTrcHigh, "Reduced xpos buffer size to: " << m_xpos.size());
    }
    if (m_zpos.size() >= 2 * (*m_SAStatNumSamples)) {
        m_zpos.erase(m_zpos.begin(), m_zpos.begin() + *m_SAStatNumSamples);
        istd_TRC(istd::eTrcHigh, "Reduced zpos buffer size to: " << m_zpos.size());
    }

    // add the retrieved sample to the data buffer
    m_sum.push_back(*m_SumSA);
    m_xpos.push_back(*m_XPosSA);
    m_zpos.push_back(*m_ZPosSA);
}

/**
 * This method first moves the old data to beginning and then appends the new
 * data at the end.
 */
void LiberaSignalAttr<LiberaSAHistory>::Refill(
    std::vector<Tango::DevDouble> &a_buff, Tango::DevDouble *a_attr)
{
    istd_TRC(istd::eTrcDetail, "Refill: " << a_buff.size());
    if (a_buff.size() == 0) {
        return;
    }
    if (a_buff.size() >= *m_SAStatNumSamples) {
        // erase old data
        a_buff.erase(a_buff.begin(), a_buff.end() - *m_SAStatNumSamples);
        istd_TRC(istd::eTrcMed, "Buffer too big, reduced to: " << a_buff.size());
    }
    else {
        // copy old data at buffer begin
        std::copy(a_attr + a_buff.size(), a_attr + *m_SAStatNumSamples, a_attr);
    }
    // copy new data after old
    std::copy(a_buff.begin(), a_buff.end(), a_attr + *m_SAStatNumSamples - a_buff.size());
}

/**
 * Implementation of statistical calculations over the current data.
 */
void LiberaSignalAttr<LiberaSAHistory>::Statistics(const Tango::DevDouble *a_buff,
    Tango::DevDouble &a_mean, Tango::DevDouble &a_rms, Tango::DevDouble &a_peak)
{
    a_mean = 0.0;
    if (m_valid == 0) {
        a_rms = 0.0;
        a_peak = 0.0;
        return;
    }
    // use just the m_valid samples at buffer end
    Tango::DevDouble hi_peak = a_buff[*m_SAStatNumSamples - m_valid];
    Tango::DevDouble lo_peak = hi_peak;
    Tango::DevDouble msqr = 0.0;
    for (size_t i(*m_SAStatNumSamples - m_valid); i < *m_SAStatNumSamples; ++i) {
        a_mean += a_buff[i];
        msqr += a_buff[i] * a_buff[i];
        if (hi_peak < a_buff[i]) {
            hi_peak = a_buff[i];
        }
        else if (lo_peak > a_buff[i]) {
            lo_peak = a_buff[i];
        }
    }
    a_mean /= m_valid;
    if (m_valid == 1) {
        a_rms = 0.0;
    }
    else {
        a_rms = sqrt((msqr - a_mean * a_mean) / (m_valid - 1));
    }
    a_peak = hi_peak - lo_peak;

    // convert units mm to um
    a_rms *= 1e3;
    a_peak *= 1e3;
}
