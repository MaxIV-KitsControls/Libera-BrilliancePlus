/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaAttr.h 18372 2012-12-19 13:43:52Z tomaz.beltram $
 */

#ifndef LIBERA_ATTR_H
#define LIBERA_ATTR_H

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <tango.h>
#pragma GCC diagnostic warning "-Wold-style-cast"

#include "LiberaBrilliancePlus.h"

#include <istd/trace.h>
#include <mci/mci.h>
#include <mci/node.h>

class LiberaClient;

/*******************************************************************************
 * Base abstract interface class, with unique access key handle.
 */
class LiberaAttr {
public:
    LiberaAttr() : m_client(NULL) {}
    virtual ~LiberaAttr() {};
    void EnableNotify(LiberaClient *a_client) { m_client = a_client; }
    void Notify();
    virtual void Read(mci::Node &a_root) = 0;
    virtual bool IsEqual(Tango::DevDouble *&) { return false; }
    virtual bool IsEqual(Tango::DevLong *&) { return false; }
    virtual bool IsEqual(Tango::DevULong *&) { return false; }
    virtual bool IsEqual(Tango::DevShort *&) { return false; }
    virtual bool IsEqual(Tango::DevUShort *&) { return false; }
    virtual bool IsEqual(Tango::DevBoolean *&) { return false; }
    /**
     *  reader and writer functions for specific atribute handling
     */
    static Tango::DevDouble NM2MM(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        int32_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val/1e6;
    }
    static Tango::DevDouble INT2DBL(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        int32_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val;
    }
    static void DBL2INT(mci::Node &a_root, const std::string &a_path, const Tango::DevDouble a_val) {
        istd_FTRC();
        int32_t val;
        if (a_val < LONG_MIN) {
            val = LONG_MIN;
        }
        else {
            if (a_val > LONG_MAX) {
                val = LONG_MAX;
            }
            else {
                val = a_val;
            }
        }
        a_root.GetNode(mci::Tokenize(a_path)).Set(val);
    }
    static Tango::DevLong ULONG2LONG(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        uint32_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val < LONG_MAX ? val : LONG_MAX;
    }
    static void LONG2ULONG(mci::Node &a_root, const std::string &a_path, const Tango::DevLong a_val) {
        istd_FTRC();
        uint32_t val = a_val > 0 ? a_val : 0;
        a_root.GetNode(mci::Tokenize(a_path)).Set(val);
    }
    static Tango::DevLong ULL2LONG(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        uint64_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val % 2^32;
    }
    static Tango::DevShort ULL2SHORT(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        uint64_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val % 2^16;
    }
    /**
     * reader and writer for negated bool value
     */
    static Tango::DevBoolean NEGATE(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        bool val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return !val;
    }
    static void NEGATE(mci::Node &a_root, const std::string &a_path, const Tango::DevBoolean a_val) {
        istd_FTRC();
        bool val(!a_val);
        a_root.GetNode(mci::Tokenize(a_path)).Set(val);
    }
    /**
     * conversion for firsts enum 0 => false, other => true
     */
    static Tango::DevBoolean ENUM2BOOL(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        int64_t val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val;
    }
    static void BOOL2ENUM(mci::Node &a_root, const std::string &a_path, const Tango::DevBoolean a_val) {
        istd_FTRC();
        int64_t val(a_val);
        a_root.GetNode(mci::Tokenize(a_path)).Set(val);
    }
    /**
     * DSCMode specific conversion for adjust and type subnodes.
     * 0 : disabled => adjust = false
     * 1 : unity    => adjust = true, type = unity(0)
     * 2 : auto     => adjust = true, type = adjusted(1)
     */
    static Tango::DevShort DSC2SHORT(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        bool enabled;
        Tango::DevShort res(0);
        a_root.GetNode(mci::Tokenize(a_path + ".adjust")).Get(enabled);
        if (enabled) {
            res = 1;
            int64_t type;
            a_root.GetNode(mci::Tokenize(a_path+".type")).Get(type);
            if (type != 0) {
                res = 2;
            }
        }
        return res;
    }
    static void SHORT2DSC(mci::Node &a_root, const std::string &a_path, const Tango::DevShort a_val) {
        istd_FTRC();
        bool enabled(a_val != 0);
        a_root.GetNode(mci::Tokenize(a_path+".adjust")).Set(enabled);
        int64_t type(a_val == 1 ? 0 : 1);
        a_root.GetNode(mci::Tokenize(a_path+".type")).Set(type);
    }
    static Tango::DevShort FAN2SHORT(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        double min;
        a_root.GetNode(mci::Tokenize(a_path + "front")).Get(min);
        double val;
        a_root.GetNode(mci::Tokenize(a_path + "middle")).Get(val);
        if (val < min) {
            min = val;
        }
        a_root.GetNode(mci::Tokenize(a_path + "rear")).Get(val);
        if (val < min) {
            min = val;
        }
        return min;
    }
    static Tango::DevShort DBL2SHORT(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        double val;
        a_root.GetNode(mci::Tokenize(a_path)).Get(val);
        return val;
    }
    static Tango::DevLong CPU2LONG(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        double user;
        a_root.GetNode(mci::Tokenize(a_path + ".ID_4.value")).Get(user);
        double kernel;
        a_root.GetNode(mci::Tokenize(a_path + ".ID_5.value")).Get(kernel);
        return user + kernel;
    }
    static Tango::DevLong MEM2LONG(mci::Node &a_root, const std::string &a_path) {
        istd_FTRC();
        double total;
        a_root.GetNode(mci::Tokenize(a_path + ".ID_0.value")).Get(total);
        double used;
        a_root.GetNode(mci::Tokenize(a_path + ".ID_1.value")).Get(used);
        return total - used;
    }
private:
    LiberaClient *m_client;
};

#endif //LIBERA_ATTR_H
