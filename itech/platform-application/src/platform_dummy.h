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

class platformDummy: public virtual iapp::LiberaApplication {
public:
	platformDummy();
	virtual ~platformDummy();

	virtual const std::string &ApplicationName() const;

protected:
	void OnRegistryAdd(ireg::TreeNodePtr &parent);
	void OnPreInit();
	void OnPostInit();
	void OnPostConfig();
	void OnShutdown();
	virtual iapp::LiberaBoard *CreateBoard(const bmc::Board* a_board);
	void EventThread();
	bool set_param(const double &a_val);
	bool get_param(double &a_val) const;

private:
	/* Dummy internal parameters */
	int m_exitThread1;

	double m_fan_left_rear;
	double m_fan_left_middle;
	double m_fan_left_front;
	double m_fan_right_rear;
	double m_fan_right_middle;
	double m_fan_right_front;

	double m_id0;
	double m_id1;
	double m_id2;
	double m_id3;
	double m_id4;
	double m_id5;




	std::thread m_thread1;

	ireg::TreeNodePtr m_nodeid0;
	ireg::TreeNodePtr m_nodeid1;
	ireg::TreeNodePtr m_nodeid2;
	ireg::TreeNodePtr m_nodeid3;
	ireg::TreeNodePtr m_nodeid4;
	ireg::TreeNodePtr m_nodeid5;

	ireg::TreeNodePtr m_nodefan_left_rear;
	ireg::TreeNodePtr m_nodefan_left_middle;
	ireg::TreeNodePtr m_nodefan_left_front;

	ireg::TreeNodePtr m_nodefan_right_rear;
	ireg::TreeNodePtr m_nodefan_right_middle;
	ireg::TreeNodePtr m_nodefan_right_front;
};

#endif // LIBERA_DUMMY_H
