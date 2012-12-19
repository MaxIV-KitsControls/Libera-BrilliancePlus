/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignalAttr.h 18368 2012-12-18 14:55:03Z tomaz.beltram $
 */

#ifndef LIBERA_SIGNAL_ATTR_H
#define LIBERA_SIGNAL_ATTR_H

#include <mutex>
#include <functional>

#include <mci/mci.h>
#include <mci/mci_util.h>

#include <isig/signal_traits.h>
#include <isig/remote_stream.h>
#include <isig/data_on_demand_remote_source.h>

#include "LiberaSignal.h"

/**
 * Type mapping template structure.
 */
template <typename TangoType>
struct TangoToTraits;

/**
 * Define supported type mappings with template specializations.
 */
template<>
struct TangoToTraits<Tango::DevDouble> {
    typedef isig::SignalTraitsVarInt32 Type;
};

template<>
struct TangoToTraits<Tango::DevShort> {
    typedef isig::SignalTraitsVarInt16 Type;
};

/*******************************************************************************
 * Data type specific class template.
 */
template<typename TangoType>
class LiberaSignalAttr : public LiberaSignal {
public:
    typedef typename TangoToTraits<TangoType>::Type Traits;
    typedef typename isig::Array<Traits>            ClientBuffer;
    typedef typename isig::RemoteStream<Traits>     RStream;
    typedef isig::DataOnDemandRemoteSource<Traits>  RSource;
    typedef typename RStream::Client                StreamClient;
    typedef typename RSource::Client                DodClient;

    template <typename ... Ts>
    LiberaSignalAttr(const char *a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize,
        Ts & ... ts)
      :  LiberaSignal(a_path, a_length, a_enabled, a_bufSize),
         m_updated(false)
    {
        Add(ts...);
        Alloc();
    }
    virtual ~LiberaSignalAttr()
    {
        istd_FTRC();
        // TODO: protect race with Transpose call, stop update thread
        Free();
    }
    virtual void Realloc(size_t a_length)
    {
        istd_FTRC();
        std::lock_guard<std::mutex> l(m_data_x);
        Free();
        SetLength(a_length);
        m_buf->Resize(GetLength());
        Alloc();
    }
private:
    void Alloc()
    {
        istd_FTRC();
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            attr = new TangoType[GetLength()];
        }
    }
    void Free()
    {
        istd_FTRC();
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            delete [] attr;
        }
    }

    virtual void Initialize(mci::Node &a_node)
    {
        istd_FTRC();

        m_signal = mci::CreateRemoteSignal(a_node);
        if (m_signal->AccessType() == isig::eAccessStream) {
            m_stream = std::dynamic_pointer_cast<RStream>(m_signal);
            m_streamClient = std::make_shared<StreamClient>(m_stream.get(), "stream_client");
            m_buf =  std::make_shared<ClientBuffer>(m_streamClient->CreateBuffer(GetLength()));
            if (m_streamClient->Open() != isig::eSuccess) {
                throw istd::Exception("Failed to open stream!");
            }
        }
        else if (m_signal->AccessType() == isig::eAccessDataOnDemand) {

            isig::AccessMode_e mode(isig::eModeDodNow); //TODO: accessor methods
            size_t readSize(GetLength()); // number of atoms to be read on event

            m_dod = std::dynamic_pointer_cast<RSource>(m_signal);
            m_dodClient = std::make_shared<DodClient>(m_dod, "dod_client", m_dod->GetTraits());
            m_buf = std::make_shared<ClientBuffer>(m_dodClient->CreateBuffer(GetLength()));

            if (m_dodClient->Open(mode, readSize, GetOffset()) != isig::eSuccess) {
                throw istd::Exception("Failed to open dod!");
            }
        }
        else {
            throw istd::Exception("Unsupported signal access mode.");
        }
    }

    virtual void GetData()
    {
        istd_FTRC();
        std::lock_guard<std::mutex> l(m_data_x);
        if (!m_updated) {
            return;
        }
        if (m_buf->GetLength() != GetLength()) {
            istd_TRC(istd::eTrcLow, "Buffer size changed while reading stream.");
            return;
        }
        for (size_t i(0); i != m_columns.size(); ++i) {
            TangoType *&attr = m_columns[i].get();
            for (size_t j(0); j < GetLength(); ++j) {
                attr[j] = (*m_buf)[j][i];
            }
        }
        istd_TRC(istd::eTrcMed, "Data copied, buffer size: "
            << m_buf->GetLength());
        m_updated = false;
    }

    void UpdateStream()
    {
        std::lock_guard<std::mutex> l(m_data_x);
        if (m_streamClient->Read(*m_buf) == isig::eSuccess) {
            m_updated = true;
            istd_TRC(istd::eTrcMed, "Stream data read, buffer size: "
                << m_buf->GetLength());
        }
        else {
            // disable signal
            throw istd::Exception("Failed to read stream!");
        }
    }

    void UpdateDod()
    {
        std::lock_guard<std::mutex> l(m_data_x);
        if (!m_updated) {
            size_t offset(0);
            isig::SignalMeta signal_meta;

            // TODO: optimize using MetaBufferPtr
            if (m_dodClient->Read(*m_buf, signal_meta, offset) == isig::eSuccess) {
                m_updated = true;
                istd_TRC(istd::eTrcMed, "Dod data read, buffer size: "
                    << m_buf->GetLength());
            }
            else {
                // disable signal
                throw istd::Exception("Failed to read DoD!");
            }
        }
    }

    virtual void UpdateSignal()
    {
        istd_FTRC();

        if (m_signal->AccessType() == isig::eAccessStream) {
            UpdateStream();
        }
        else if (m_signal->AccessType() == isig::eAccessDataOnDemand) {
            UpdateDod();
        }
        else {
            throw istd::Exception("Unsupported signal access mode.");
        }
    }

    inline void Add() {};
    template <typename ... Ts>
    inline void Add(TangoType *&t, Ts & ... ts)
    {
        m_columns.push_back(std::ref(t));
        Add(ts...);
    }
    isig::SignalSourceSharedPtr   m_signal;
    std::shared_ptr<RStream>      m_stream;
    std::shared_ptr<StreamClient> m_streamClient;
    std::shared_ptr<RSource>      m_dod;
    std::shared_ptr<DodClient>    m_dodClient;
    std::shared_ptr<ClientBuffer> m_buf;
    std::vector<std::reference_wrapper<TangoType *> > m_columns;
    bool       m_updated;
    std::mutex m_data_x;
};
#endif //LIBERA_SIGNAL_ATTR_H
