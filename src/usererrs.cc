#include <string.h>
#include <errno.h>
#include <lmerr.h>
#include "usererrs.h"

const char* APISnag::message()
{
  switch (_code)
  {
// TODO maybe: put these in code # order
    case ERROR_ACCESS_DENIED:
      return "access is denied to current user";
    case ERROR_NETWORK_UNREACHABLE:
      return "the network location cannot be reached (connection)";
    case ERROR_HOST_UNREACHABLE:
      return "the network location cannot be reached (remote host)";
    case ERROR_PROTOCOL_UNREACHABLE:
      return "the network location cannot be reached (protocol problem)";
    case ERROR_BAD_NETPATH:
      return "the network path was not found (check hostname)";
    case ERROR_INVALID_LEVEL:
      return "the requested information level is not supported";
    case ERROR_INVALID_NAME:
      return "the given hostname is malformed";
// TODO: since the following case can come from ConvertSidToStringSid *or*
// from NetUserGetLocalGroups, and the likely cause is so different, we
// should build another field into the Snag class for extra information.
// Example: for ConvertSidToStringSid it might be "check destination pointer",
// while for NetUserGetLocalGroups it might be "check flags parameter"
    case ERROR_INVALID_PARAMETER:
      return "invalid parameter";
    case ERROR_NOT_ENOUGH_MEMORY:
      return "not enough memory to perform the API request";
    case NERR_BufTooSmall:
      return "the API result buffer is too small [DEVEL ERROR]";
    case NERR_DCNotFound:
      return "the domain controller could not be found";
    case NERR_InternalError:
      return "an internal error occurred";
    case NERR_InvalidComputer:
      return "the given hostname is unrecognized";
    case NERR_UserNotFound:
      return "the given username could not be found";
    case RPC_S_SERVER_UNAVAILABLE:
      return "no RPC server available - can't query the named host";

    // These are possible errors from ConvertSidToStringSid:
    // case ERROR_NOT_ENOUGH_MEMORY already covered above;
    // case ERROR_INVALID_PARAMETER already covered above;
    case ERROR_INVALID_SID:
      return "an invalid SID was passed to an API function";
  }
  return NULL;
}

const char* SystemSnag::message()
{
  switch (_code)
  {
    case EILSEQ:
      return "system gave a wide character that could not be converted to multibyte";
    case ENOMEM:
      return "failed to allocate memory";
  }
  return NULL;
}

const char* UsageSnag::message()
{
  switch (_code)
  {
    case EILSEQ:
      return "invalid multibyte character";
    case EMSGSIZE:
      return "given name is too long";
  }
  return NULL;
}

