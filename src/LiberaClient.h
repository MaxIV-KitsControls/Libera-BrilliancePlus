/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaClient.h 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#ifndef LIBERA_CLIENT_H
#define LIBERA_CLIENT_H

#include <mci/node.h>

#include "LiberaScalarAttr.h"
#include "LiberaLogsAttr.h"
#include "LiberaSignalAttr.h"
#include "LiberaSignalSAHistory.h"

using LiberaBrilliancePlus_ns::LiberaBrilliancePlus;

/*******************************************************************************
 * Class for handling connection to the Libera application.
 */
class LiberaClient {
public:
    LiberaClient(LiberaBrilliancePlus *a_deviceServer, std::string ip_address);
    ~LiberaClient();

    bool Connect();
    void Disconnect();
    bool IsConnected();

    void operator ()();

    /**
     *  Methods for adding different attribute types to the update list.
     *  The created object's class template is based on the attribute type used
     *  in this method call.
     *  Optional reader and writer function parameter is used for unit
     *  conversion and special node handling.
     */
    template <typename TangoType>
    void AddScalar(const std::string &a_path, TangoType *&a_attr,
        TangoType (*a_reader)(mci::Node &, const std::string &) = LiberaScalarAttr<TangoType>::DoRead,
        void (*a_writer)(mci::Node &, const std::string &, const TangoType) = LiberaScalarAttr<TangoType>::DoWrite)
    {
        m_attr.push_back(
            std::make_shared<LiberaScalarAttr<TangoType> >(a_path, a_attr, a_reader, a_writer));
    }

    /**
     * Atributes related to platform management use platform daemon registry
     * and are added to different list.
     */
    template <typename TangoType>
    void AddScalarPM(const std::string &a_path, TangoType *&a_attr,
        TangoType (*a_reader)(mci::Node &, const std::string &) = LiberaScalarAttr<TangoType>::DoRead,
        void (*a_writer)(mci::Node &, const std::string &, const TangoType) = LiberaScalarAttr<TangoType>::DoWrite)
    {
        m_attr_pm.push_back(
            std::make_shared<LiberaScalarAttr<TangoType> >(a_path, a_attr, a_reader, a_writer));
    }

    /**
     * Spectrum attribute that is not a signal needs special handling.
     */
    void AddLogsRead(Tango::DevString *&a_attr, const size_t a_size)
    {
        m_attr.push_back(
            std::make_shared<LiberaLogsAttr>(a_attr, a_size));
    }

    /**
     * Assign the LiberaBrilliancePlus object's function to be called
     * if attribute value changes.
     */
    template<typename TangoType>
    void SetNotifier(TangoType *&a_attr, void (LiberaBrilliancePlus::*a_notifier)())
    {
        for (auto i = m_attr.begin(); i != m_attr.end(); ++i) {
            if ((*i)->IsEqual(a_attr)) {
                m_notify[i->get()] = std::bind(a_notifier, m_deviceServer);
                (*i)->EnableNotify(this);
            }
        }
    }

    void Notify(LiberaAttr *a_attr);

    /**
     * Write the value to the attribute handling object.
     * Will disconnect in case of error.
     */
    template<typename TangoType>
    void UpdateScalar(TangoType *&a_attr, const TangoType a_val)
    {
        istd_FTRC();
        try {
            // could be optimized using map<>::find() if needed
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
            m_deviceServer->set_lib_error();
        }
    }

    /**
     * Create signal handling object and assign scalar attribute pointers to it.
     */
    template <typename TangoType, typename ... Ts>
    LiberaSignal * AddSignal(const std::string &a_path, const size_t a_length,
        Tango::DevBoolean *&a_enabled, Tango::DevLong *&a_bufSize,
        Ts & ... ts)
    {
        auto p = std::make_shared<LiberaSignalAttr<TangoType> >(
            a_path.c_str(), a_length, a_enabled, a_bufSize, ts...);
        m_signals.push_back(p);
        return p.get(); // Return LiberaSignal<> object address as a handle.
    }

    bool Execute(const std::string &a_path);
    bool MagicCommand(const std::string &a_path, Tango::DevVarStringArray *a_out);
private:

    void UpdateAttr();
    void Connect(mci::Node &a_root, mci::Root a_type);
    void Disconnect(mci::Node &a_root, mci::Root a_type);
    void TreeWalk(const mci::Node &a_node, Tango::DevVarStringArray *a_out);

    std::atomic<bool>   m_connected;
    std::atomic<bool>   m_running;
    std::thread         m_thread;

    LiberaBrilliancePlus *m_deviceServer; // used for changing device state

    std::string m_ip_address;

    mci::Node            m_root;
    mci::Node            m_platform;

    std::vector<std::shared_ptr<LiberaAttr> >   m_attr;    // list of attributes to be updated
    std::vector<std::shared_ptr<LiberaAttr> >   m_attr_pm; // platform list of attributes
    std::vector<std::shared_ptr<LiberaSignal> > m_signals; // list of managed signals
    std::map<LiberaAttr *, std::function<void ()> > m_notify; // map of notification callbacks
};

#endif //LIBERA_CLIENT_H
