/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaLogsAttr.cpp 18339 2012-12-14 12:09:03Z tomaz.beltram $
 */

#include "LiberaLogsAttr.h"

char c_emptyStr[] = "";

/**
 * Implementation for log read attribute of type  Spectrum of Tango::DevString.
 */
LiberaLogsAttr::LiberaLogsAttr(Tango::DevString *&a_attr, const size_t a_size)
  : LiberaAttr(),
    m_size(a_size),
    m_attr(a_attr)
{
    m_attr = new Tango::DevString[m_size];
    for (size_t i(0); i < m_size; ++i) {
        m_attr[i] = c_emptyStr;
    }
}

LiberaLogsAttr::~LiberaLogsAttr()
{
    delete [] m_attr;
}

void LiberaLogsAttr::Read(mci::Node &)
{
    //TODO
}

void LiberaLogsAttr::Write(mci::Node &, const Tango::DevString)
{
    //TODO
}

