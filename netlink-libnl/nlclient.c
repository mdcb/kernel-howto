#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "nlcommon.h"

/* attribute policy: defines which attribute has which type (e.g int, char * etc)
 * possible values defined in net/netlink.h
 */
static struct nla_policy demo_gnl_policy[DEMO_ATTR_MAX + 1] =
{
  [DEMO_ATTR1_STRING] = {.type = NLA_STRING,.maxlen = 256},
  [DEMO_ATTR2_UINT16] = {.type = NLA_U16},
  [DEMO_ATTR3_CUSTOM] = { .type = NLA_UNSPEC,.maxlen = sizeof(struct attr_custom)},
};

static int cb_handler(struct nl_msg * msg, void * arg)
{

  * (int*) arg = 123; // cbarg

  struct nlmsghdr * hdr = nlmsg_hdr(msg);
  struct genlmsghdr * gnlh = nlmsg_data(hdr);

  int valid =
    genlmsg_validate(hdr, 0, DEMO_ATTR_MAX, demo_gnl_policy);
  printf("valid %d %s\n", valid, valid ? "ERROR" : "OK");

  // one way
  struct nlattr * attrs[DEMO_ATTR_MAX + 1];

  if (genlmsg_parse(hdr, 0, attrs, DEMO_ATTR_MAX, demo_gnl_policy) < 0)
    {
      printf("genlsmg_parse ERROR\n");
    }

  else
    {
      printf("genlsmg_parse OK\n");
      printf("attr1 %s\n", nla_get_string(attrs[DEMO_ATTR1_STRING]));
      printf("attr2 %x\n", nla_get_u16(attrs[DEMO_ATTR2_UINT16]));
      struct attr_custom * cp = (struct attr_custom *) nla_data(attrs[DEMO_ATTR3_CUSTOM]);
      printf("attr3 %d %ld %f %lf\n", cp->a, cp->b, cp->c,cp->d);

    }

  // another way
  printf("gnlh->cmd %d\n", gnlh->cmd);	//--- DEMO_CMD_ECHO

  int remaining = genlmsg_attrlen(gnlh, 0);
  struct nlattr * attr = genlmsg_attrdata(gnlh, 0);

  while (nla_ok(attr, remaining))
    {
      printf("remaining %d\n", remaining);
      printf("attr @ %p\n", attr); // nla_get_string(attr)
      attr = nla_next(attr, &remaining);
    }

  nl_msg_dump(msg, stderr);

  return NL_STOP;
}

int main()
{
  struct nl_sock * sk;
  int cbarg;

  // nl_debug = 4;

  // setup netlink socket
  sk = nl_socket_alloc();
  nl_socket_disable_seq_check(sk);	// disable sequence number check
  genl_connect(sk);

  int id = genl_ctrl_resolve(sk, DEMO_FAMILY_NAME);

  struct nl_msg * msg;


  // create a messgae
  msg = nlmsg_alloc();
  genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, id, 0,	// hdrlen
                        0,	// flags
                        DEMO_CMD,	// numeric command identifier
                        DEMO_VERSION	// interface version
                       );

  nla_put_string(msg, DEMO_ATTR1_STRING, "hola");
  nla_put_u16(msg, DEMO_ATTR2_UINT16, 0xf1);

  // send it
  nl_send_auto(sk, msg);

  // handle reply
  struct nl_cb * cb = NULL;
  cb = nl_cb_alloc(NL_CB_CUSTOM);

  //nl_cb_set_all(cb, NL_CB_DEBUG, NULL, NULL);
  nl_cb_set_all(cb, NL_CB_CUSTOM, cb_handler, &cbarg);
  nl_cb_err(cb, NL_CB_DEBUG, NULL, NULL);

  int nrecv = nl_recvmsgs_report(sk, cb);

  printf("cbarg %d nrecv %d\n", cbarg, nrecv);

  // cleanup
  nlmsg_free(msg);
  nl_close(sk);
  nl_socket_free(sk);

  return 0;
}
