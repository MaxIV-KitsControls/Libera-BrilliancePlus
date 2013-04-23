/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaScalarAttr.h 18413 2013-01-09 11:53:17Z tomaz.beltram $
 */

#ifndef LIBERA_SCALAR_ATTR_H
#define LIBERA_SCALAR_ATTR_H

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include <istd/trace.h>
#include <mci/mci.h>
#include <mci/node.h>

#include "LiberaAttr.h"

/**
 * Type mapping template structure.
 */
template <typename TangoType>
struct TangoToLibera;

/**
 * Define supported type mappings with template specializations.
 */
template<>
struct TangoToLibera<Tango::DevDouble> {
    typedef double Type;
};

template<>
struct TangoToLibera<Tango::DevLong> {
    typedef int32_t Type;
};

template<>
struct TangoToLibera<Tango::DevULong> {
    typedef uint32_t Type;
};

template<>
struct TangoToLibera<Tango::DevShort> {
    typedef int32_t Type;
};

template<>
struct TangoToLibera<Tango::DevUShort> {
    typedef uint32_t Type;
};

template<>
struct TangoToLibera<Tango::DevBoolean> {
    typedef bool Type;
};

/*******************************************************************************
 * Class for mapping between types, storing values and ireg access.
 * The a_attr reference passed in constructor is used for updating
 * attribute value via Read/Write methods.
 * Can be used just for storing value if the ireg node path is empty.
 */
template <typename TangoType>
class LiberaScalarAttr : public LiberaAttr {
public:
    typedef typename TangoToLibera<TangoType>::Type LiberaType;

    /**
     * Assign the attribute pointer reference and allocate attribute memory.
     * The attributes may not have the associated ireg node so the path can
     * be empty
     */
    LiberaScalarAttr(const std::string a_path, TangoType *&a_attr,
        TangoType (*a_reader)(mci::Node &, const std::string &),
        void (*a_writer)(mci::Node &, const std::string &, const TangoType))
      : LiberaAttr(),
        m_attr(a_attr),
        m_path(a_path),
        m_reader(a_reader),
        m_writer(a_writer)
    {
        m_attr = new TangoType;
        if (m_path.empty()) {
            *m_attr = 0;
        }
    }
    virtual ~LiberaScalarAttr()
    {
        delete m_attr;
        istd_TRC(istd::eTrcDetail, "Destroyed scalar attribute for: " << m_path);
    }

    /**
     * Default reader function gets value from registry.
     */
    static TangoType DoRead(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        LiberaType val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val;
    }

    /**
     * Call the reader function and notify client if value has changed.
     */
    virtual void Read(mci::Node &a_root) {
        istd_FTRC();
        if (!m_path.empty()) {
            istd_TRC(istd::eTrcDetail, "Read from node: " << m_path);
            TangoType val = m_reader(a_root, m_path);
            // poor man's notification client
            // could also use mci::NotificationClient
            if (*m_attr != val) {
                *m_attr = val;
                Notify();
            }
        }
    }

    /**
     * Default writer function puts value to registry node.
     */
    static void DoWrite(mci::Node &a_root, const std::string &a_path, const TangoType a_val) {
        istd_FTRC();
        LiberaType val(a_val);
        a_root.GetNode(mci::Tokenize(a_path)).Set(val);
    }

    /**
     * Call the writer function.
     */
    void Write(mci::Node &a_root, const TangoType a_val) {
        if (!m_path.empty()) {
            m_writer(a_root, m_path, a_val);
            *m_attr = a_val;
        }
    }

    /**
     * The attribute memory address is used as handle since it doesn't change
     * in object's lifetime.
     */
    bool IsEqual(TangoType *&a_attr) { return a_attr == m_attr; }

private:
    TangoType *&m_attr;
    const std::string m_path;
    TangoType (*m_reader)(mci::Node &, const std::string &);
    void (*m_writer)(mci::Node &, const std::string &, const TangoType);
};

#endif //LIBERA_SCALAR_ATTR_H
