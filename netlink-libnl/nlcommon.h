#ifndef NLCOMMON_H
#define NLCOMMON_H

#define DEMO_FAMILY_NAME "DEMO_NETLINK"

struct attr_custom {
	int a;
	long b;
	float c;
	double d;
};

enum {
	DEMO_ATTR1_STRING = 1,
	DEMO_ATTR2_UINT16,
	DEMO_ATTR3_CUSTOM,
	__DEMO_ATTR_MAX,
};
#define DEMO_ATTR_MAX (__DEMO_ATTR_MAX - 1)

enum {
	DEMO_CMD = 1,
};

#define DEMO_VERSION 1

#endif
