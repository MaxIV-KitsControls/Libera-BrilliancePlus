/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaLogsAttr.h 18339 2012-12-14 12:09:03Z tomaz.beltram $
 */

#ifndef LIBERA_LOGS_ATTR_H
#define LIBERA_LOGS_ATTR_H

#include "LiberaAttr.h"

/*******************************************************************************
 * Derived log read attribute class.
 */

class LiberaLogsAttr : public LiberaAttr {
public:
    LiberaLogsAttr(Tango::DevString *&a_attr, const size_t a_size);
    virtual ~LiberaLogsAttr();

    virtual void Read(mci::Node &a_root);
    void Write(mci::Node &a_root, const Tango::DevString a_val);

private:
    size_t             m_size;
    Tango::DevString *&m_attr;
};

#endif //LIBERA_LOGS_ATTR_H
