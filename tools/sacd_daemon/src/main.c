/**
 * SACD Ripper - http://code.google.com/p/sacd-ripper/
 *
 * Copyright (c) 2010-2011 by respective authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <io/pad.h>

#include <sysutil/msg.h>
#include <sysutil/sysutil.h>
#include <sysutil/disc.h>

#include <sysmodule/sysmodule.h>

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/thread.h> 
#include <sys/spu.h> 

#include <net/net.h>
#include <net/netctl.h> 

#include <sys/storage.h>
#include <patch-utils.h>
#include <utils.h>

#include "rsxutil.h"
#include "exit_handler.h"
#include "server.h"
#ifdef ENABLE_LOGGING
#include "output_device.h"
#endif

#include <logging.h>
#include <version.h>

#define MAX_PHYSICAL_SPU               6
#define MAX_RAW_SPU                    1

static int dialog_action = 0;
static int bd_contains_sacd_disc = -1;      // information about the current disc
static int bd_disc_changed = -1;            // when a disc has changed this is set to zero
static int loaded_modules = 0;

static int load_modules(void)
{
    int ret;

    ret = sysModuleLoad(SYSMODULE_FS);
    if (ret != 0)
        return ret;
    else
        loaded_modules |= 1;

    ret = sysModuleLoad(SYSMODULE_IO);
    if (ret != 0)
        return ret;
    else
        loaded_modules |= 2;

    ret = sysModuleLoad(SYSMODULE_GCM_SYS);
    if (ret != 0)
        return ret;
    else
        loaded_modules |= 4;

    return ret;
}

static int unload_modules(void)
{
    if (loaded_modules & 4)
        sysModuleUnload(SYSMODULE_GCM_SYS);

    if (loaded_modules & 2)
        sysModuleUnload(SYSMODULE_IO);

    if (loaded_modules & 1)
        sysModuleUnload(SYSMODULE_FS);

    return 0;
}

int file_simple_save(const char *filePath, void *buf, unsigned int fileSize)
{
    int      ret;
    int      fd;
    uint64_t writelen;

    if (buf == NULL)
    {
        LOG(lm_main, LOG_ERROR, ("buffer is null\n"));
    }

    ret = sysFsOpen(filePath, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd, NULL, 0);
    if ((ret != 0))        // && (ret != EPERM) ){
    {
        LOG(lm_main, LOG_ERROR, ("file %s open error : 0x%x\n", filePath, ret));
        return -1;
    }

    ret = sysFsWrite(fd, buf, fileSize, &writelen);
    if (ret != 0 || fileSize != writelen)
    {
        LOG(lm_main, LOG_ERROR, ("file %s read error : 0x%x\n", filePath, ret));
        sysFsClose(fd);
        return -1;
    }

    ret = sysFsClose(fd);
    if (ret != 0)
    {
        LOG(lm_main, LOG_ERROR, ("file %s close error : 0x%x\n", filePath, ret));
        return -1;
    }

    return 0;
}

static void dialog_handler(msgButton button, void *usrData)
{
    switch (button)
    {
    case MSG_DIALOG_BTN_OK:
        dialog_action = 1;
        break;
    case MSG_DIALOG_BTN_NO:
    case MSG_DIALOG_BTN_ESCAPE:
        dialog_action = 2;
        break;
    case MSG_DIALOG_BTN_NONE:
        dialog_action = -1;
        break;
    default:
        break;
    }
}

int patch_lv1_ss_services(void)
{
    install_new_poke();

    // Try to map lv1
    if (!map_lv1()) 
    {
        remove_new_poke();
        return -1;
    }

    lv1poke(0x0016f3b8, 0x7f83e37860000000ULL); // 0x7f83e378f8010098ULL
    lv1poke(0x0016f3dc, 0x7f85e37838600001ULL); // 0x7f85e3784bfff0e5ULL
    lv1poke(0x0016f454, 0x7f84e3783be00001ULL); // 0x7f84e37838a10070ULL
    lv1poke(0x0016f45c, 0x9be1007038600000ULL); // 0x9be1007048005fa5ULL

    remove_new_poke();

    // unmap lv1
    unmap_lv1(); 

    return 0;
}

int unpatch_lv1_ss_services(void)
{
    install_new_poke();

    // Try to map lv1
    if (!map_lv1()) 
    {
        remove_new_poke();
        return -1;
    }

    lv1poke(0x0016f3b8, 0x7f83e378f8010098ULL);
    lv1poke(0x0016f3dc, 0x7f85e3784bfff0e5ULL);
    lv1poke(0x0016f454, 0x7f84e37838a10070ULL);
    lv1poke(0x0016f45c, 0x9be1007048005fa5ULL);

    remove_new_poke();

    // unmap lv1
    unmap_lv1(); 

    return 0;
}

int patch_syscall_864(void)
{
    const uint64_t addr          = 0x80000000002D7820ULL; // 3.55 addr location
    uint8_t        access_rights = lv2peek(addr) >> 56;
    if (access_rights == 0x20)
    {
        lv2poke(addr, (uint64_t) 0x40 << 56);
    }
    else if (access_rights != 0x40)
    {
        return -1;
    }
    return 0;
}

static void bd_eject_disc_callback(void)
{
    bd_contains_sacd_disc = -1;
    bd_disc_changed       = -1;
}

static void bd_insert_disc_callback(uint32_t disc_type, char *title_id)
{
    bd_disc_changed = 1;

    if (disc_type == SYS_DISCTYPE_PS3)
    {
        // cannot do anything with a PS3 disc..
        bd_contains_sacd_disc = 0;
    }
    else
    {
        // unknown disc
        bd_contains_sacd_disc = 1;
    }
}

void main_loop(void)
{
    int client_connected;
    msgType              dialog_type;
    char                 *message = (char *) malloc(512);

    // did the disc change?
    if (bd_contains_sacd_disc && bd_disc_changed)
    {
        bd_contains_sacd_disc = 0;
    }
    
    // by default we have no user controls
    dialog_type = (MSG_DIALOG_NORMAL | MSG_DIALOG_DISABLE_CANCEL_ON);

    if (!bd_contains_sacd_disc)
    {
    	union net_ctl_info info;
    	
    	if(netCtlGetInfo(NET_CTL_INFO_IP_ADDRESS, &info) == 0)
    	{
       		sprintf(message, "              SACD Daemon %s\n\n"
       		                 "Status: Active\n"
       		                 "IP Address: %s (port 2002)\n"
       		                 "Client: %s\n"
       		                 "Disc: %s",
    			SACD_RIPPER_VERSION_STRING, info.ip_address, 
    			(is_client_connected() ? "connected" : "none"),
    			(bd_disc_changed == -1 ? "empty" : "inserted"));
    	}
    	else
    	{
    		sprintf(message, "No active network connection was detected.\n\nPress OK to refresh.");
            dialog_type |= MSG_DIALOG_BTN_TYPE_OK;
    	} 
    }

    msgDialogOpen2(dialog_type, message, dialog_handler, NULL, NULL);

    dialog_action         = 0;
    bd_disc_changed       = 0;
    client_connected      = is_client_connected();
    while (!dialog_action && !user_requested_exit() && bd_disc_changed == 0 && client_connected == is_client_connected())
    {
        sysUtilCheckCallback();
        flip();
    }
    msgDialogAbort();

    free(message);
}

int main(int argc, char *argv[])
{
    int     ret;
    void    *host_addr = memalign(1024 * 1024, HOST_SIZE);
    msgType dialog_type;
	sys_ppu_thread_t id; // start server thread

    load_modules();

    init_logging();

	netInitialize();
	netCtlInit(); 

    // Initialize SPUs
    LOG(lm_main, LOG_DEBUG, ("Initializing SPUs\n"));
    ret = sysSpuInitialize(MAX_PHYSICAL_SPU, MAX_RAW_SPU);
    if (ret != 0)
    {
        LOG(lm_main, LOG_ERROR, ("sysSpuInitialize failed: %d\n", ret));
        goto quit;
    }

    init_screen(host_addr, HOST_SIZE);
    ioPadInit(7);

    ret = initialize_exit_handlers();
    if (ret != 0)
        goto quit;

    // remove patch protection
    remove_protection();

    ret = patch_lv1_ss_services();
    if (ret < 0)
    {
        dialog_type = (MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
        msgDialogOpen2(dialog_type, "ERROR: Couldn't patch lv1 services, returning to the XMB.\nMake sure you are running a firmware like 'kmeaw' which allows patching!", dialog_handler, NULL, NULL);

        dialog_action = 0;
        while (!dialog_action && !user_requested_exit())
        {
            sysUtilCheckCallback();
            flip();
        }
        msgDialogAbort();

        goto quit;
    }

    // patch syscall 864 to allow drive re-init
    ret = patch_syscall_864();
    if (ret < 0)
    {
        dialog_type = (MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);
        msgDialogOpen2(dialog_type, "ERROR: Couldn't patch syscall 864, returning to the XMB.\nMake sure you are running a firmware like 'kmeaw' which allows patching!", dialog_handler, NULL, NULL);

        dialog_action = 0;
        while (!dialog_action && !user_requested_exit())
        {
            sysUtilCheckCallback();
            flip();
        }
        msgDialogAbort();

        goto quit;
    }

    // reset & re-authenticate the BD drive
    sys_storage_reset_bd();
    sys_storage_authenticate_bd();

    ret = sysDiscRegisterDiscChangeCallback(&bd_eject_disc_callback, &bd_insert_disc_callback);

#ifdef ENABLE_LOGGING
    // poll for an output_device
    poll_output_devices();

    if (output_device) 
    {
        char file_path[100];
        sprintf(file_path, "%s/daemon_log.txt", output_device);
        set_log_file(file_path);
    }
#endif

	sysThreadCreate(&id, listener_thread, NULL, 1500, 0x400, 0, "listener");

    while (1)
    {
        // main loop
        main_loop();

        // break out of the loop when requested
        if (user_requested_exit())
            break;
    }

    ret = sysDiscUnregisterDiscChangeCallback();

 quit:

    unpatch_lv1_ss_services();

    destroy_logging();
	netDeinitialize();
    unload_modules();
    
    free(host_addr);

    return 0;
}
