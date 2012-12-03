/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaClient.h 18277 2012-12-03 12:02:45Z tomaz.beltram $
 */

#ifndef LIBERA_CLIENT_H
#define LIBERA_CLIENT_H

#include <mci/node.h>

#include "LiberaAttr.h"
#include "LiberaSignal.h"

/*******************************************************************************
 * Class for handling connection to the Libera application.
 */
class LiberaClient {
public:
    LiberaClient(Tango::Device_4Impl *a_deviceServer, const std::string &a_board);

    bool Connect();
    void Disconnect();

    // methods for adding different attribute types to the update list
    template <typename TangoType>
    void AddScalar(const char *a_path, TangoType *&a_attr)
    {
        std::string path(a_path);
        // Add board prefix or else leave attribute node path empty.
        if (!path.empty()) {
            path = m_board + path;
        }
        m_attr.push_back(
            std::make_shared<LiberaScalarAttr<TangoType> >(path, a_attr));
    }

    void AddLogsRead(Tango::DevString *&a_attr, const size_t a_size)
    {
        m_attr.push_back(
            std::make_shared<LogsReadAttr>(a_attr, a_size));
    }

    template<typename TangoType>
    void UpdateScalar(TangoType *a_attr, const TangoType a_val)
    {
        istd_FTRC();
        try {
            // TODO: optimize using map<>::find()
            for (auto i = m_attr.begin(); i != m_attr.end(); ++i) {
                if ((*i)->IsEqual(a_attr)) {
                    auto p = std::dynamic_pointer_cast<LiberaScalarAttr<TangoType> >(*i);
                    p->Write(m_root, a_val);
                }
            }
        }
        catch (istd::Exception e)
        {
            istd_TRC(istd::eTrcLow, "Exception thrown while writing to node!");
            istd_TRC(istd::eTrcLow, e.what());
            // let the server know it
            m_deviceServer->set_state(Tango::OFF);
        }
    }

    template <typename TangoType, typename ... Ts>
    LiberaSignalBase * AddSignal(const char *a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize,
        Ts & ... ts)
    {
        std::string path(a_path);
        // Add board prefix or else leave attribute node path empty.
        if (!path.empty()) {
            path = m_board + path;
        }
        auto p = std::make_shared<LiberaSignal<TangoType> >(
            path.c_str(), a_length, a_enabled, a_bufSize, ts...);
        m_signals.push_back(p);
        return p.get(); // Return LiberaSignal<> object address as a handle.
    }
    void Update();
    void Enable(LiberaSignalBase *a_signal);
    void Disable(LiberaSignalBase *a_signal);

private:

    Tango::Device_4Impl *m_deviceServer;
    std::string          m_board;
    mci::Node            m_root;
    std::vector<std::shared_ptr<LiberaAttr> >   m_attr;    // list of attributes to be updated
    std::vector<std::shared_ptr<LiberaSignalBase> > m_signals; // list of managed signals
};

#endif //LIBERA_CLIENT_H
