#ifndef __BWCOACHDATA_H
#define __BWCOACHDATA_H

typedef struct _stBWCOACH_ACTION_DESC
{
	int number;
	char *desc;
	int duration;
} stBWCOACH_ACTION_DESC;

class BWCoachData
{
public:
	static int GetDuration(int i);
	static const char *GetMessage(int i);
	static int GetCount();
};

#endif
