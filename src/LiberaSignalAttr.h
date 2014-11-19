/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignalAttr.h 18420 2013-01-10 14:26:03Z tomaz.beltram $
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

    /**
     * Implementation of signal class allocates memory for spectrum attributes.
     * The order of attributes must match that of the signal atom components.
     */
    template <typename ... Ts>
    LiberaSignalAttr(const char *a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize,
        Ts & ... ts)
      :  LiberaSignal(a_path, a_length, a_enabled, a_bufSize),
         m_offset(0),
         m_updated(false)
    {
        Add(ts...);
        Alloc();
    }

    virtual ~LiberaSignalAttr()
    {
        istd_FTRC();
        // Protect race with UpdateSignal call, stop update thread first.
        Stop();
        Free();
    }

    /**
     * Public method for changing acquisition buffer size.
     */
    virtual void Realloc(size_t a_length)
    {
        istd_FTRC();
        // No locking needed here since changing only user side buffers that
        // are updated on user request.
        Free();
        SetLength(a_length);
        Alloc();
    }

    /**
     * Check if new data is available.
     */
    bool IsUpdated()
    {
        return m_updated;
    }

    /**
     * Invalidate acquired data.
     */
    void ClearUpdated()
    {
        m_updated = false;
    }

    virtual void SetOffset(int32_t a_offset)
    {
        m_offset = a_offset;
    }

protected:
    /**
     * Method for copying buffer data.
     */
    virtual void GetData()
    {
        istd_FTRC();
        // Skip data copy if not updated.
        if (!m_updated) {
            return;
        }
        std::lock_guard<std::mutex> l(m_data_x);
        if (m_buf->GetLength() != GetLength()) {
            istd_TRC(istd::eTrcMed, "Buffer size changed while reading signal."
                << " Was: " << m_buf->GetLength() << ", is: " << GetLength());
            m_buf->Resize(GetLength());
            // Invalidate buffer so that next acquisition can adjust it.
            m_updated = false;
            return;
        }
        for (size_t i(0); i != m_columns.size(); ++i) {
            TangoType *&attr = m_columns[i].get();
            for (size_t j(0); j < GetLength(); ++j) {
                attr[j] = (*m_buf)[j][i];
            }
        }
        istd_TRC(istd::eTrcHigh, "Data copied, buffer size: "
            << m_buf->GetLength());
        m_updated = false;
    }

    /**
     * Method for differentiating between stream and data on demand (dod)
     * access type. It is called from base class internal thread or public
     * method.
     */
    virtual void UpdateSignal()
    {
        istd_FTRC();

        if (!m_signal) {
            throw istd::Exception("Signal not initialized yet.");
        }
        else if (m_signal->AccessType() == isig::eAccessStream) {
            UpdateStream();
        }
        else if (m_signal->AccessType() == isig::eAccessDataOnDemand) {
            UpdateDod();
        }
        else {
            throw istd::Exception("Unsupported signal access mode.");
        }
    }

private:

    virtual int32_t GetOffset()
    {
        return m_offset;
    }

    /**
     * Allocate data buffers of same length for each spectrum attribute.
     */
    void Alloc()
    {
        istd_FTRC();
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            size_t len(GetLength());
            attr = new TangoType[len];
            std::fill(attr, attr + len, TangoType(0));
        }
        istd_TRC(istd::eTrcDetail, "New size: " << GetLength());
    }

    /**
     * Release allocated memory.
     */
    void Free()
    {
        istd_FTRC();
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            delete [] attr;
        }
    }

    /**
     * Connects to the signal and creates client object depending on the type
     * of signal and its data acquisition structure.
     */
    virtual void Initialize(mci::Node &a_node)
    {
        istd_FTRC();

        m_signal = mci::CreateRemoteSignal(a_node);
        if (m_signal->AccessType() == isig::eAccessStream) {
            m_stream = std::dynamic_pointer_cast<RStream>(m_signal);
            if (m_streamClient && m_streamClient->IsOpen()) {
                m_streamClient->Close();
            }
            m_streamClient = std::make_shared<StreamClient>(m_stream.get(), "stream_client");
            m_buf =  std::make_shared<ClientBuffer>(m_streamClient->CreateBuffer(GetLength()));
            if (m_streamClient->Open() != isig::eSuccess) {
                throw istd::Exception("Failed to open stream!");
            }
        }
        else if (m_signal->AccessType() == isig::eAccessDataOnDemand) {

            m_dod = std::dynamic_pointer_cast<RSource>(m_signal);
            if (m_dodClient && m_dodClient->IsOpen()) {
                m_dodClient->Close();
            }
            m_dodClient = std::make_shared<DodClient>(m_dod, "dod_client", m_dod->GetTraits());
            m_buf = std::make_shared<ClientBuffer>(m_dodClient->CreateBuffer(GetLength()));

            // No open here, since the dod client is opened just before read
        }
        else {
            throw istd::Exception("Unsupported signal access mode.");
        }
    }

    /**
     * Update internal data buffer using stream client.
     */
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

    /**
     * Update internal data buffer using dod client.
     */
    void UpdateDod()
    {
        // skip read if not copied since last update
        if (!m_updated) {

            size_t readSize(GetLength()); // number of atoms to be read on event
            size_t offset(0); // TODO: use ExternalTriggerDelay here?
            isig::SignalMeta signal_meta;

            std::lock_guard<std::mutex> l(m_data_x);
            if (m_dodClient->Open(GetMode(), readSize, offset) != isig::eSuccess) {
                istd_TRC(istd::eTrcLow, "Error opening dod in mode: " << GetMode());
                throw istd::Exception("Failed to open dod!");
            }

            // Can be optimized using MetaBufferPtr if necessary.
            if (m_dodClient->Read(*m_buf, signal_meta, GetOffset()) == isig::eSuccess) {
                m_updated = true;
                istd_TRC(istd::eTrcMed, "Dod data read, buffer size: "
                    << m_buf->GetLength());
            }
            else {
                // disable signal
                istd_TRC(istd::eTrcLow, "Error reading dod in mode: " << GetMode());
                throw istd::Exception("Failed to read dod!");
            }
            m_dodClient->Close();
        }
        else {
            if (GetMode() != isig::eModeDodNow) {
                // avoid busy loop when not reading
                ::usleep(100000);
            }
        }
    }

    void Add(TangoType *&t)
    {
        m_columns.push_back(std::ref(t));
    };

    /**
     * Add variable number of spectrum attribute pointer references.
     */
    template <typename ... Ts>
    void Add(TangoType *&t, Ts & ... ts)
    {
        Add(t);
        Add(ts...);
    }
    int32_t                       m_offset;
    isig::SignalSourceSharedPtr   m_signal;
    std::shared_ptr<RStream>      m_stream;
    std::shared_ptr<StreamClient> m_streamClient;
    std::shared_ptr<RSource>      m_dod;
    std::shared_ptr<DodClient>    m_dodClient;
    std::vector<std::reference_wrapper<TangoType *> > m_columns;
    std::atomic<bool>             m_updated;
    std::mutex                    m_data_x; // protects m_buf access
    std::shared_ptr<ClientBuffer> m_buf;
};

#endif //LIBERA_SIGNAL_ATTR_H
