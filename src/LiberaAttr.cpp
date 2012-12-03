/*
 * Copyright (c) 2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: LiberaAttr.cpp 18277 2012-12-03 12:02:45Z tomaz.beltram $
 */

#include "LiberaAttr.h"

/**
 * Implementation for log read attribute of type  Spectrum of Tango::DevString.
 */
LogsReadAttr::LogsReadAttr(Tango::DevString *&a_attr, const size_t a_size)
  : LiberaAttr(a_attr),
    m_size(a_size),
    m_attr(a_attr)
{
    m_attr = new Tango::DevString[m_size];
    for (int i(0); i < m_size; ++i) {
        m_attr[i] = "";
    }
}

LogsReadAttr::~LogsReadAttr()
{
    delete [] m_attr;
}

void LogsReadAttr::Read(mci::Node &a_root)
{
    //TODO
}

void LogsReadAttr::Write(mci::Node &a_root, const Tango::DevString a_val)
{
    //TODO
}

