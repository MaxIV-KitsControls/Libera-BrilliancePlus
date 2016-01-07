/*
 * Copyright (c) 2008-2012 Instrumentation Technologies
 * All Rights Reserved.
 *
 * $Id: $
 */

#ifndef LIBERA_DUMMY_H
#define LIBERA_DUMMY_H

#include "iapp/libera_application.h"

/*--------------------------------------------------------------------------------------*/
/* Dummy Application class                                                              */

class liberaDummy: public virtual iapp::LiberaApplication {
public:
	liberaDummy();
	virtual ~liberaDummy();

	virtual const std::string &ApplicationName() const;

protected:
	void OnRegistryAdd(ireg::TreeNodePtr &parent);
	void OnPreInit();
	void OnPostInit();
	void OnPostConfig();
	void OnShutdown();
	virtual iapp::LiberaBoard *CreateBoard(const bmc::Board* a_board);
	void EventThread();

private:
	bool IlkStatusReset();
    bool RegExec();
	void RegistryAddTiming(const char *a_name);
    void RegistryAddRaf(const char *a_name);
    void RegistryAddApplication();
	/* Dummy internal parameters */
	int m_exitThread1;
	std::thread m_thread1;

	std::vector<isig::SignalSourceSharedPtr> m_signals;
	uint64_t m_ilkStatus;
	int32_t *m_refTest;
};

#endif // LIBERA_DUMMY_H
