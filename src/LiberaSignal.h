/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.h 18277 2012-12-03 12:02:45Z tomaz.beltram $
 */

#ifndef LIBERA_SIGNAL_H
#define LIBERA_SIGNAL_H

#include <thread>
#include <chrono>
#include <functional>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 4)
    #include <atomic>
#else
    #include <cstdatomic>
#endif

#include <isig/array.h>
#include <mci/node.h>

#include <isig/signal_traits.h>
#include <isig/remote_stream.h>
#include <isig/data_on_demand_remote_source.h>

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
 * Base signal class for reading streams and dod.
 */
class LiberaSignalBase {
public:
    LiberaSignalBase(const char *a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize);
    virtual ~LiberaSignalBase();

    void Enable(mci::Node &a_root);
    void Disable();

    void operator ()();

protected:
    size_t GetLength();
    void SetLength(size_t a_length);

private:
    void Update();
    virtual void UpdateStream(isig::SignalSourceSharedPtr aSignal) = 0;
    virtual void UpdateDod(isig::SignalSourceSharedPtr aSignal) = 0;

    std::atomic<bool>   m_running;
    std::thread         m_thread;
    Tango::DevBoolean *&m_enabled;
    Tango::DevLong    *&m_length; // length of each column

    const std::string m_path;
    mci::Node m_root;
};

/*******************************************************************************
 * Data type specific class template.
 */
template<typename TangoType>
class LiberaSignal : public LiberaSignalBase {
public:
    typedef typename TangoToTraits<TangoType>::Type Traits;
    typedef typename isig::RemoteStream<Traits> RStream;
    typedef isig::DataOnDemandRemoteSource<Traits> RSource;
    template <typename ... Ts>
    LiberaSignal(const char *a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize,
        Ts & ... ts)
      :  LiberaSignalBase(a_path, a_length, a_enabled, a_bufSize)
    {
        Add(ts...);
        Alloc();
    }
    virtual ~LiberaSignal()
    {
        Free();
    }
    void Realloc(size_t a_length)
    {
        Free();
        SetLength(a_length);
        Alloc();
    }
private:
    void Alloc()
    {
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            attr = new TangoType[GetLength()];
        }
    }
    void Free()
    {
        for (auto i = m_columns.begin(); i != m_columns.end(); ++i) {
            TangoType *&attr(*i);
            delete [] attr;
        }
    }

    void Transpose(isig::Array<Traits> &a_array)
    {
        for (size_t i(0); i != m_columns.size(); ++i) {
            for (size_t j(0); j < a_array.GetLength(); ++j) {
                TangoType *&attr = m_columns[i].get();
                attr[j] = a_array[j][i];
            }
        }
    }

    virtual void UpdateStream(isig::SignalSourceSharedPtr aSignal)
    {
        auto stream = std::dynamic_pointer_cast<RStream>(aSignal);
        typename RStream::Client cl(stream.get(), "stream_client");
        typename RStream::Buffer buf(cl.CreateBuffer(GetLength()));

        if (cl.Open() == isig::eSuccess) {
            if (cl.Read(buf) == isig::eSuccess) {
                istd_TRC(istd::eTrcLow, "Stream data read, buffer size: "
                    << buf.GetLength());
                Transpose(buf);
            }
            cl.Close();
        }
    }

    virtual void UpdateDod(isig::SignalSourceSharedPtr aSignal)
    {
        auto dod = std::dynamic_pointer_cast<RSource>(aSignal);
        typename RSource::Client cl(dod->CreateClient("dod_client"));
        typename RSource::Buffer buf(cl.CreateBuffer(GetLength()));

        isig::AccessMode_e mode(isig::eModeDodPosition);
        size_t readSize(GetLength()); // number of atoms to be read on event
        size_t offset(0);
        isig::SignalMeta signal_meta;

        if (cl.Open(mode, readSize, offset) == isig::eSuccess) {
            if (cl.Read(buf, signal_meta, offset) == isig::eSuccess) {
                istd_TRC(istd::eTrcLow, "Dod data read, buffer size: "
                    << buf.GetLength());
                Transpose(buf);
            }
            cl.Close();
        }
    }

    inline void Add() {};
    template <typename ... Ts>
    inline void Add(TangoType *&t, Ts & ... ts)
    {
        m_columns.push_back(std::ref(t));
        Add(ts...);
    }
    std::vector<std::reference_wrapper<TangoType *> > m_columns;
};
#endif //LIBERA_SIGNAL_H
