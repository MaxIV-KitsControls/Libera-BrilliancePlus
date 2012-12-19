/*
 * Copyright (c) 2008-2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: simul_stream.h 18316 2012-12-11 09:01:14Z tomaz.beltram $
 */

#include <stdint.h>
#include <math.h>
#include <fstream>

#include "isig/stream.h"
#include "isig/data_on_demand_source.h"
#include "isig/circular_buffer_controller_base.h"

// TODO: get values from registry nodes
const size_t c_bufferLen(10);

const int32_t c_signalAmp(10000);
const double c_signalNoise(100);

/*****************************************************************************/

class TestCircularBufferController:
    public isig::CircularBufferControllerBase,
    public isig::Clonable<TestCircularBufferController> {

public:
    TestCircularBufferController(
        size_t                  a_size,
        const uint64_t&         a_frequency)
    : isig::CircularBufferControllerBase(a_size, a_frequency) {
        isig::LMT lmt;
        GetCurrentLmt(lmt);
        SetStartLmt(lmt);
    }

    virtual ~TestCircularBufferController() {}


private:

    virtual isig::SuccessCode_e GetEventLmt(isig::LMT& a_lmt) const
    {
        ::sleep(1);
        return GetCurrentLmt(a_lmt);
    }
    virtual isig::SuccessCode_e GetCurrentLmt(isig::LMT& a_lmt) const
    {
        // get time
        a_lmt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return isig::eSuccess;
    }

};

/*****************************************************************************/

/**
 * ADC Data on Demand structures
 */
struct AdcAtom {
    int16_t ChannelA, ChannelB, ChannelC, ChannelD;
};

typedef isig::SignalTraits<int16_t, AdcAtom, c_bufferLen> AdcTraits;

template<>
const AdcTraits::Names AdcTraits::names =
    {{"ChannelA", "ChannelB", "ChannelC", "ChannelD"}};

class AdcBuffer: public isig::RandomAccessBuffer {
public:
    AdcBuffer() : RandomAccessBuffer(c_bufferLen*sizeof(int16_t)*4) {}
    virtual ~AdcBuffer() {}
protected:
    virtual size_t QueryByteSize()
    {
        size_t s(c_bufferLen*sizeof(int16_t)*4);
        return s;
    }
    virtual void OpenInput() {}
    virtual void CloseInput() {}
    virtual std::size_t  ReadInput(
        void* a_buf,
        size_t a_count,
        size_t a_position)
    {
        istd_TRC(istd::eTrcLow, "a_count: " << a_count);

        static const double c_maxLevel(10000); // sampled signal level
        static const double c_decay = 0.9; // decay factor
        static const double c_rise = 0.3; // rise factor
        static const double c_fo = 0.95; //sample frequency offset

        int16_t *buf = static_cast<int16_t *>(a_buf);
        size_t cnt = a_count/sizeof(int16_t)/4;

        // random phase noise
        double phA = M_PI*rand()/RAND_MAX;
        double phB = M_PI*rand()/RAND_MAX;
        double phC = M_PI*rand()/RAND_MAX;
        double phD = M_PI*rand()/RAND_MAX;

        double signal = 0;
        for (size_t i(0); i < cnt; ++i) {
            signal *= c_decay;
            if ( 10 < i && i < 15) { // pulse interval
                signal += (1-signal)*c_rise;
            }
            buf[4*i+0] = signal*sin(i*M_PI/2*c_fo+phA)*c_maxLevel;
            buf[4*i+1] = signal*sin(i*M_PI/2*c_fo+phB)*c_maxLevel;
            buf[4*i+2] = signal*sin(i*M_PI/2*c_fo+phC)*c_maxLevel;
            buf[4*i+3] = signal*sin(i*M_PI/2*c_fo+phD)*c_maxLevel;
        }
        return a_count;
    }
};

isig::RandomAccessBufferPtr g_testAdc(std::make_shared<AdcBuffer>());

class SimulAdc : public isig::DataOnDemandSource<AdcTraits> {
public:
    typedef isig::DataOnDemandSource<AdcTraits> BaseDoD;
    SimulAdc() : BaseDoD("simul-adc", g_testAdc)
    {
        auto bc = std::make_shared<TestCircularBufferController>(c_bufferLen, 1);
        auto dc = std::dynamic_pointer_cast<isig::DodPositionController>(bc);
        istd_TRC(istd::eTrcLow, "Configuring AdcSignalNew");
        this->SetPositionController(dc);
        this->Open();
    }
};

/*****************************************************************************/

/**
 * DDC Data on Demand Raw structures
 */

const size_t c_dscChannels(4);

struct IQAtom {
    int32_t I;
    int32_t Q;
};

typedef std::array<IQAtom, c_dscChannels> TbtDdcRawAtom;

typedef isig::SignalTraits<int32_t,  TbtDdcRawAtom>    TbtDdcRawTraits;

template<> const TbtDdcRawTraits::Names TbtDdcRawTraits::names =
    {{"Ia", "Qa", "Ib", "Qb", "Ic", "Qc", "Id", "Qd"}};

class DdcRawBuffer: public isig::RandomAccessBuffer {
public:
    DdcRawBuffer() : RandomAccessBuffer(c_bufferLen*sizeof(int32_t)*8) {}
    virtual ~DdcRawBuffer() {}
protected:
    virtual size_t QueryByteSize()
    {
        size_t s(c_bufferLen*sizeof(int32_t)*8);
        return s;
    }
    virtual void OpenInput() {}
    virtual void CloseInput() {}
    virtual std::size_t  ReadInput(
        void* a_buf,
        size_t a_count,
        size_t a_position)
    {
        istd_TRC(istd::eTrcLow, "a_count: " << a_count);

        static const double c_maxLevel(10000); // sampled signal level
        static const double c_decay = 0.9; // decay factor
        static const double c_rise = 0.3; // rise factor
        static const double c_fo = 0.95; //sample frequency offset

        int32_t *buf = static_cast<int32_t *>(a_buf);
        size_t cnt = a_count/sizeof(int32_t)/8;

        // random phase noise
        double phA = M_PI*rand()/RAND_MAX;
        double phB = M_PI*rand()/RAND_MAX;
        double phC = M_PI*rand()/RAND_MAX;
        double phD = M_PI*rand()/RAND_MAX;

        double signal = 0;
        for (size_t i(0); i < cnt; ++i) {
            signal *= c_decay;
            if ( 10 < i && i < 15) { // pulse interval
                signal += (1-signal)*c_rise;
            }
            buf[8*i+0] = signal*sin(i*M_PI/2*c_fo+phA)*c_maxLevel;
            buf[8*i+1] = signal*cos(i*M_PI/2*c_fo+phA)*c_maxLevel;
            buf[8*i+2] = signal*sin(i*M_PI/2*c_fo+phB)*c_maxLevel;
            buf[8*i+3] = signal*cos(i*M_PI/2*c_fo+phB)*c_maxLevel;
            buf[8*i+4] = signal*sin(i*M_PI/2*c_fo+phC)*c_maxLevel;
            buf[8*i+5] = signal*cos(i*M_PI/2*c_fo+phC)*c_maxLevel;
            buf[8*i+6] = signal*sin(i*M_PI/2*c_fo+phD)*c_maxLevel;
            buf[8*i+7] = signal*cos(i*M_PI/2*c_fo+phD)*c_maxLevel;
        }
        return a_count;
    }
};

isig::RandomAccessBufferPtr g_testDdcRaw(std::make_shared<DdcRawBuffer>());

class SimulDdcRaw : public isig::DataOnDemandSource<TbtDdcRawTraits> {
public:
    typedef isig::DataOnDemandSource<TbtDdcRawTraits> BaseDoD;
    SimulDdcRaw() : BaseDoD("simul-ddc_raw", g_testDdcRaw)
    {
        auto bc = std::make_shared<TestCircularBufferController>(c_bufferLen, 1);
        auto dc = std::dynamic_pointer_cast<isig::DodPositionController>(bc);
        istd_TRC(istd::eTrcLow, "Configuring DdcRawSignalNew");
        this->SetPositionController(dc);
        this->Open();
    }
};

/*****************************************************************************/

/**
 * DDC Data on Demand Synthetic structures
 */

struct TbtDdcSynthAtom {
    int32_t v_a;
    int32_t v_b;
    int32_t v_c;
    int32_t v_d;
    int32_t sum;
    int32_t q;
    int32_t x;
    int32_t y;
};

typedef isig::SignalTraits<int32_t,  TbtDdcSynthAtom>   TbtDdcSynthTraits;

template<> const TbtDdcSynthTraits::Names TbtDdcSynthTraits::names =
    {{"Va", "Vb", "Vc", "Vd", "Sum", "Q", "X", "Y"}};

class DdcSynthBuffer: public isig::RandomAccessBuffer {
public:
    DdcSynthBuffer() : RandomAccessBuffer(c_bufferLen*sizeof(int32_t)*8) {}
    virtual ~DdcSynthBuffer() {}
protected:
    virtual size_t QueryByteSize()
    {
        size_t s(c_bufferLen*sizeof(int32_t)*8);
        return s;
    }
    virtual void OpenInput() {}
    virtual void CloseInput() {}
    virtual std::size_t  ReadInput(
        void* a_buf,
        size_t a_count,
        size_t a_position)
    {
        istd_TRC(istd::eTrcLow, "a_count: " << a_count);

        static const double c_maxLevel(10000); // sampled signal level

        int32_t *buf = static_cast<int32_t *>(a_buf);
        size_t cnt = a_count/sizeof(int32_t)/8;

        double signal = 1;
        for (size_t i(0); i < cnt; ++i) {
            double signalA = signal+rand()/1e2/RAND_MAX;
            double signalB = signal+rand()/1e2/RAND_MAX;
            double signalC = signal+rand()/1e2/RAND_MAX;
            double signalD = signal+rand()/1e2/RAND_MAX;
            double signalSum = signalA+signalB+signalC+signalD;
            if (signalSum <= 0) {
                signalSum = 1;
            }
            buf[8*i+0] = signalA*c_maxLevel;
            buf[8*i+1] = signalB*c_maxLevel;
            buf[8*i+2] = signalC*c_maxLevel;
            buf[8*i+3] = signalD*c_maxLevel;
            buf[8*i+4] = signalSum*c_maxLevel;
            buf[8*i+5] = (signalA - signalB + signalC - signalD)/signalSum*c_maxLevel;
            buf[8*i+6] = (signalA - signalB - signalC + signalD)/signalSum*c_maxLevel;
            buf[8*i+7] = (signalA + signalB - signalC - signalD)/signalSum*c_maxLevel;
        }
        return a_count;
    }
};

isig::RandomAccessBufferPtr g_testDdcSynth(std::make_shared<DdcSynthBuffer>());

class SimulDdcSynth : public isig::DataOnDemandSource<TbtDdcSynthTraits> {
public:
    typedef isig::DataOnDemandSource<TbtDdcSynthTraits> BaseDoD;
    SimulDdcSynth() : BaseDoD("simul-ddc_synth", g_testDdcSynth)
    {
        auto bc = std::make_shared<TestCircularBufferController>(c_bufferLen, 1);
        auto dc = std::dynamic_pointer_cast<isig::DodPositionController>(bc);
        istd_TRC(istd::eTrcLow, "Configuring DdcSynthSignalNew");
        this->SetPositionController(dc);
        this->Open();
    }
};

/*****************************************************************************/

/**
 * SA Stream structures
 */

struct SaAtom {
    int32_t va;
    int32_t vb;
    int32_t vc;
    int32_t vd;
    int32_t sum;
    int32_t q;
    int32_t x;
    int32_t y;
    uint64_t timestamp;
    uint32_t reserved[5];
    uint32_t status;
};

typedef isig::SignalTraits<int32_t,  SaAtom>      SaTraits;

template<> const SaTraits::Names SaTraits::names =
    {{"Va", "Vb", "Vc", "Vd", "Sum", "Q", "X", "Y", "LMT_l", "LMT_h", "r1", "r2", "r3", "r4", "r5", "status"}};

class SaStream : public isig::Stream<SaTraits> {

public:

    SaStream()
    : isig::Stream<SaTraits>("sa-stream", SaTraits()),
      m_sleep(0.1),
      m_index(0)
    {
    };

    virtual ~SaStream()
    {
        this->ShutDown();
    }

    double m_sleep;

private:

    int32_t                  m_index;
    std::mutex               m_mutex;
    std::condition_variable  m_cond;

    virtual void CloseInput()
    {
        m_cond.notify_all();
    };

    typedef isig::Stream<SaTraits> _Base;
    virtual isig::SuccessCode_e ReadInput(_Base::Buffer &a_buf) {
        istd_FTRC();

        std::unique_lock<std::mutex> lk(m_mutex);

        if (m_cond.wait_for(lk, std::chrono::duration<double>(m_sleep))
            != std::cv_status::timeout) {
            // Condition has been notified,
            // it must happened because the input has been closed.
            return isig::eClosed;
        }

        size_t len(a_buf.GetLength());

        for (size_t i(0); i<len; ++i) {
            istd_TRC(istd::eTrcLow, "i: " << i);
            SaAtom &a(a_buf[i]);
            memset(&a, 0, sizeof(SaAtom));
            a.va = c_signalAmp + c_signalNoise * rand() / RAND_MAX;
            a.vb = c_signalAmp + c_signalNoise * rand() / RAND_MAX;
            a.vc = c_signalAmp + c_signalNoise * rand() / RAND_MAX;
            a.vd = c_signalAmp + c_signalNoise * rand() / RAND_MAX;
            a.sum = a.va + a.vb + a.vc + a.vd;
            if (a.sum > 0) {
                a.q = 1e7 * (a.va - a.vb + a.vc - a.vd) / a.sum;
                a.x = 1e7 * (a.va - a.vb - a.vc + a.vd) / a.sum;
                a.y = 1e7 * (a.va + a.vb - a.vc - a.vd) / a.sum;
            }
            a.timestamp = m_index++;
        }

        return isig::eSuccess;
    }
};
