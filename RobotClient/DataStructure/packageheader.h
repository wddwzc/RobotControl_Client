#ifndef PACKAGEHEADER_H
#define PACKAGEHEADER_H

#include "DeviceConnection/socketcmd.h"

struct PackageHeader {
    CmdType msg_type;
    CmdType msg_data;
    unsigned int msg_len;
};

#endif // PACKAGEHEADER_H
