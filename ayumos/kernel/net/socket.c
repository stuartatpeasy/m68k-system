/*
    Socket implementation

    Part of ayumos


    (c) Stuart Wallace, April 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/net/socket.h>


/*
	socket_init() - allocate memory to hold socket array.
*/
s32 socket_init()
{
	return SUCCESS;
}


/*
    socket_create() - create a socket object.
*/
s32 socket_create(s32 domain, s32 type, s32 protocol)
{
    UNUSED(domain);
	UNUSED(type);
	UNUSED(protocol);


    return SUCCESS;
}

#endif /* WITH_NETWORKING */
