/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaAttr.h 18277 2012-12-03 12:02:45Z tomaz.beltram $
 */

#ifndef LIBERA_ATTR_H
#define LIBERA_ATTR_H

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include <istd/trace.h>
#include <mci/mci.h>
#include <mci/node.h>

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
struct TangoToLibera<Tango::DevUShort> {
    typedef uint32_t Type;
};

template<>
struct TangoToLibera<Tango::DevShort> {
    typedef int32_t Type;
};

template<>
struct TangoToLibera<Tango::DevBoolean> {
    typedef bool Type;
};

/*******************************************************************************
 * Base abstract interface class, with unique access key handle.
 */
class LiberaAttr {
public:
    typedef void * Handle;
    LiberaAttr(Handle a_handle) : m_handle(a_handle) {}
    virtual void Read(mci::Node &a_root) = 0;
    virtual ~LiberaAttr() {};
    bool IsEqual(Handle a_handle) { return a_handle == m_handle; }
private:
    Handle m_handle;
};

/*******************************************************************************
 * Class for mapping between types, storing values and ireg access.
 * The a_attr reference passed in constructor is used for updating
 * attribute value via Read/Write methods..
 */
template <typename TangoType>
class LiberaScalarAttr : public LiberaAttr {
public:
    typedef typename TangoToLibera<TangoType>::Type LiberaType;
    LiberaScalarAttr(const std::string a_path, TangoType *&a_attr)
      : LiberaAttr(a_attr),
        m_attr(a_attr),
        m_path(a_path)
    {
        m_attr = new TangoType;
    }
    virtual ~LiberaScalarAttr()
    {
        delete m_attr;
    };

    virtual void Read(mci::Node &a_root) {
        istd_FTRC();
        //TODO: lock
        if (m_path.empty()) {
            m_val = 0;
        }
        else {
            istd_TRC(istd::eTrcDetail, "Read from node: " << m_path);
            a_root.GetNode(mci::Tokenize(m_path)).Get(m_val);
        }
        *m_attr = m_val;
    }

    void Write(mci::Node &a_root, const TangoType a_val) {
        //TODO: lock
        m_val = a_val;
        a_root.GetNode(mci::Tokenize(m_path)).Set(m_val);
        *m_attr = m_val;
    }

private:
    LiberaType m_val;
    TangoType *&m_attr;
    const std::string m_path;
};

/*******************************************************************************
 * Derived log read attribute class.
 */

class LogsReadAttr : public LiberaAttr {
public:
    LogsReadAttr(Tango::DevString *&a_attr, const size_t a_size);
    virtual ~LogsReadAttr();

    virtual void Read(mci::Node &a_root);
    void Write(mci::Node &a_root, const Tango::DevString a_val);

private:
    size_t             m_size;
    Tango::DevString *&m_attr;
};

#endif //LIBERA_ATTR_H
