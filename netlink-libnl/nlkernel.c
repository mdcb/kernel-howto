#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "nlcommon.h"

static struct nla_policy demo_gnl_policy[DEMO_ATTR_MAX + 1] = {
	[DEMO_ATTR1_STRING] = { .type = NLA_NUL_STRING, .len = 256 },	/* variable length NULL terminated string */
	[DEMO_ATTR2_UINT16] = { .type = NLA_U16 }, // optional .len = 2
	[DEMO_ATTR3_CUSTOM] = { .len = sizeof(struct attr_custom) }, // optional .type = NLA_UNSPEC
};

static struct genl_family demo_gnl_family = {
	.id = GENL_ID_GENERATE,	// genetlink should generate an id
	.hdrsize = 0,
	.name = DEMO_FAMILY_NAME,
	.version = DEMO_VERSION,
	.maxattr = DEMO_ATTR_MAX,
};

int demo_cmd(struct sk_buff *skb_2, struct genl_info *info)
{
	struct nlattr *na;
	struct sk_buff *skb;
	int rc = 0;
	void *msg_head;
	char *attr_str;
    u16 attr_u16;
	struct attr_custom cp;

	printk("got demo_cmd\n");

	if (info == NULL) {
		goto out;
	}

	na = info->attrs[DEMO_ATTR1_STRING];
	if (na) {
		attr_str = (char *)nla_data(na);
		if (attr_str == NULL) {
			printk("error while receiving data\n");
		}
		else {
			printk("attr1: %s\n", attr_str);
		}
	}
	else {
		printk("no attr1\n");
	}

	na = info->attrs[DEMO_ATTR2_UINT16];
	if (na) {
		attr_u16 = nla_get_u16(na);
		printk("attr2: %x\n", attr_u16);
	}
	else {
		printk("no attr2\n");
	}

	/* send message back */
	/* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE */
	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (skb == NULL) {
		goto out;
	}

	/* create the message */
	msg_head =
	    genlmsg_put(skb, 0, info->snd_seq + 1, &demo_gnl_family, 0,
			DEMO_CMD);

	if (msg_head == NULL) {
		rc = -ENOMEM;
		goto out;
	}

	rc |= nla_put_string(skb, DEMO_ATTR1_STRING,"world");
	rc |= nla_put_u16(skb, DEMO_ATTR2_UINT16, 0x1f);
	cp.a = 1;
	cp.b = 2;
	cp.c = 3.0;
	cp.d = 4.0;
	rc |= nla_put(skb, DEMO_ATTR3_CUSTOM, sizeof(struct attr_custom), &cp);

	if (rc != 0) {
		goto out;
	}

	/* finalize the message */
	genlmsg_end(skb, msg_head);

	/* send the message back */
	rc = genlmsg_unicast(&init_net, skb, info->snd_portid);

	if (rc != 0) {
		goto out;
	}

	return 0;

 out:
	printk("an error occured\n");

	return -1;
}

/* command mapping */
static struct genl_ops doc_exmpl_gnl_ops_echo[] = {
	{
	.cmd = DEMO_CMD,
	.flags = 0,
	.policy = demo_gnl_policy,
	.doit = demo_cmd,
	.dumpit = NULL,
	},
};

static int __init nlk_init(void)
{
	int rc;
	printk("nlk_init\n");

	/* register new family */
	rc = genl_register_family_with_ops(&demo_gnl_family, doc_exmpl_gnl_ops_echo);

	if (rc != 0) {
		printk("register family with ops: %i\n", rc);
		genl_unregister_family(&demo_gnl_family);
		goto failure;
	}

	return 0;

 failure:
	printk
	    ("an error occured\n");
	return -1;

}

static void __exit nlk_exit(void)
{
	int ret;
	printk("nlk_exit\n");

	/* unregister the family */
	ret = genl_unregister_family(&demo_gnl_family);

	if (ret != 0) {
		printk("unregister family %i\n", ret);
	}
}

module_init(nlk_init);
module_exit(nlk_exit);
MODULE_LICENSE("GPL");
