/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaSignal.h 18368 2012-12-18 14:55:03Z tomaz.beltram $
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

/*******************************************************************************
 * Base signal class for reading streams and dod.
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
    void SetOffset(int32_t a_offset);
    virtual void Realloc(size_t a_length) = 0;

    void operator ()();
    void Update();
    virtual void GetData() = 0;

protected:
    int32_t GetOffset();
    size_t GetLength();
    void SetLength(size_t a_length);

private:
    virtual void Initialize(mci::Node &a_node) = 0;
    virtual void UpdateSignal() = 0;

    std::atomic<bool>   m_running;
    std::thread         m_thread;
    uint32_t            m_period;
    int32_t             m_offset;
    Tango::DevBoolean *&m_enabled;
    Tango::DevLong    *&m_length; // length of each column

    const std::string m_path;
    mci::Node m_root;
};

#endif //LIBERA_SIGNAL_H
