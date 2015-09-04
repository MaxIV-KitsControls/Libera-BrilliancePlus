/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.h 18420 2013-01-10 14:26:03Z tomaz.beltram $
 */

#ifndef LIBERA_SIGNAL_H
#define LIBERA_SIGNAL_H

#include <thread>
#include <mutex>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 4)
    #include <atomic>
#else
    #include <cstdatomic>
#endif

#include <mci/node.h>

typedef void (*SignalCallback)(void *);

/*******************************************************************************
 * Base abstract signal class for reading streams and dod.
 */
class LiberaSignal {
public:
    LiberaSignal(const std::string &a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize);
    virtual ~LiberaSignal();

    bool Connect(mci::Node &a_root);
    void Enable();
    void Disable();
    void SetPeriod(uint32_t a_period);
    void operator ()();
    void Update();
    void SetMode(isig::AccessMode_e  a_mode);

    void SetNotifier(SignalCallback a_callback, void *a_arg);

    // interface functions for the derived class
    virtual void SetOffset(int32_t a_offset) = 0;
    virtual void Realloc(size_t a_length) = 0;
    virtual bool IsUpdated() = 0;
    virtual void ClearUpdated() = 0;
    virtual void GetData() = 0;

protected:
    virtual int32_t    GetOffset() = 0;
    isig::AccessMode_e GetMode();
    size_t GetLength();
    void   SetLength(size_t a_length);
    void   Stop();
private:
    virtual void Initialize(mci::Node &a_node) = 0;
    virtual void UpdateSignal() = 0;

    std::atomic<bool>   m_running;
    std::thread         m_thread;
    uint32_t            m_period;
    Tango::DevBoolean *&m_enabled;
    Tango::DevLong    *&m_length; // length of each column
    bool                m_connected;
    isig::AccessMode_e  m_mode;

    const std::string m_path;
    mci::Node m_root;

    SignalCallback m_callback;
    void *m_callback_arg;

};

#endif //LIBERA_SIGNAL_H
