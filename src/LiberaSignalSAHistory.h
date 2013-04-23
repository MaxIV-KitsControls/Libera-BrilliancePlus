/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignalSAHistory.h 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#ifndef LIBERA_SIGNAL_SA_HISTORY_H
#define LIBERA_SIGNAL_SA_HISTORY_H

#include <mutex>

#include "LiberaSignalAttr.h"

/*******************************************************************************
 * Special class template for SA history and statistics.
 */

class LiberaSAHistory; // actually its just a type for template specialization

template<>
class LiberaSignalAttr<LiberaSAHistory> : public LiberaSignalAttr<Tango::DevDouble> {
public:
    LiberaSignalAttr(const char *a_path, const size_t a_length,
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
        Tango::DevDouble *&a_SumMeanSA_read);
    ~LiberaSignalAttr();
private:
    virtual void Realloc(size_t a_length);
    virtual void GetData();
    virtual void UpdateSignal();
    void Refill(std::vector<Tango::DevDouble> &a_buff, Tango::DevDouble *a_attr);
    void Statistics(const Tango::DevDouble *a_buff,
        Tango::DevDouble &a_mean, Tango::DevDouble &a_rms, Tango::DevDouble &a_peak);
    Tango::DevLong *m_SASize; // underlaying SA stream length
    Tango::DevLong   *&m_SAStatNumSamples; // history buffer length

    // references of the SA stream
    Tango::DevDouble *&m_SumSA;
    Tango::DevDouble *&m_XPosSA;
    Tango::DevDouble *&m_ZPosSA;

    // history attributes
    Tango::DevDouble *&m_XPosSAHistory;
    Tango::DevDouble *&m_ZPosSAHistory;
    Tango::DevDouble *&m_SumSAHistory;

    // statistic attributes
    Tango::DevDouble m_XMeanPosSA;
    Tango::DevDouble m_ZMeanPosSA;
    Tango::DevDouble m_XRMSPosSA;
    Tango::DevDouble m_ZRMSPosSA;
    Tango::DevDouble m_XPeakPosSA;
    Tango::DevDouble m_ZPeakPosSA;
    Tango::DevDouble m_SumMeanSA;

    // history buffer
    std::vector<Tango::DevDouble> m_xpos;
    std::vector<Tango::DevDouble> m_zpos;
    std::vector<Tango::DevDouble> m_sum;
    std::mutex m_data_x;
    size_t m_valid;
};

#endif //LIBERA_SIGNAL_SA_HISTORY_H
