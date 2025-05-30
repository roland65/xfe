/*
 * Extended from original code found here: https://github.com/tom5760/usermount
 * Copyright (c) 2014 Tom Wambold <tom5760@gmail.com>
 * Copyright (c) 2014 Marius L. Jøhndal <mariuslj@ifi.uio.no>
 * Copyright (c) 2019 Moritz Bruder <muesli4 at gmail dot com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Xfe automounter, requires udisks2 (which depends on gio / gvfs)

// Notes about MTP devices
//   udisks2 does not handle MTP devices, so we implement a custom mechanism for these devices
//   We use 'gio mount -li' to detect MTP devices and 'gio mount' to mount them
//   However, we need to prevent automounting an MTP device once the user has unmounted it
//   And we also need to automount the device when it is plugged again 
//   This is why we use a vector of MTP devices with some flags to handle these cases correctly


#include <sys/stat.h>
#include <errno.h>

#include <iostream>
#include <sstream>
#include <optional>
#include <string>
#include <vector>

#define UDISKS_API_IS_SUBJECT_TO_CHANGE
#include <udisks/udisks.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#include "../src/xfedefs.h"


// MTP device data
struct MTPDevice
{
    char name[MAX_MTP_NAME_SIZE];
    int plugged = FALSE;
    int to_automount = TRUE;
};


// Vector of MTP device data
typedef std::vector<MTPDevice>  vector_MTPDevice;
vector_MTPDevice MTP;


// If true, open an Xfe window
bool automount_open = false;


static constexpr std::string_view BLOCK_DEVICES_PATH{ "/org/freedesktop/UDisks2/block_devices/" };

struct FileSystemInfo
{
    std::string_view const dbus_path;
    UDisksFilesystem* filesystem;
    std::string_view const label;
    std::string_view const uuid;
    gboolean const mountable;
};


// Safe strcpy function (Public domain, by C.B. Falconer)
// The destination string is always null terminated
// Size sz must be equal to strlen(src)+1
size_t xf_strlcpy(char* dst, const char* src, size_t sz)
{
    const char* start = src;

    if (src && sz--)
    {
        while ((*dst++ = *src))
        {
            if (sz--)
            {
                src++;
            }
            else
            {
                *(--dst) = '\0';
                break;
            }
        }
    }
    if (src)
    {
        while (*src++)
        {
            continue;
        }
        return src - start - 1;
    }
    else if (sz)
    {
        *dst = '\0';
    }
    return 0;
}


#ifdef HAVE_LIBNOTIFY
static const int NOTIFICATION_TIMEOUT_MS = 5000; /* 5 seconds */
static const char *NOTIFICATION_APP_NAME = "xfe-automount";

static void send_notification(char *path)
{
    NotifyNotification *n;
    gchar *message;
    GError *error = NULL;

    notify_init(NOTIFICATION_APP_NAME);

    message = g_strdup_printf("Device mounted on %s", path);

    n = notify_notification_new(NOTIFICATION_APP_NAME, message, NULL);
    notify_notification_set_timeout(n, NOTIFICATION_TIMEOUT_MS);

    if (!notify_notification_show(n, &error))
    {
        fprintf(stderr, "Failed to send notification: %s\n", error->message);
        g_error_free(error);
    }

    g_free(message);
    g_object_unref(G_OBJECT(n));
}
#endif


// Return command output or error message
std::string getcommandoutput(std::string cmd)
{
    std::string data;
    FILE* stream;

    const int max_buffer = 1024;
    char buffer[max_buffer];

    cmd += " 2>&1";

    stream = popen(cmd.data(), "r");
    if (stream)
    {
        while (!feof(stream))
        {
            if (fgets(buffer, max_buffer, stream) != NULL)
            {
                data += buffer;
            }
        }
        pclose(stream);
    }
    return data;
}


// Get info about DBUS object
std::optional<FileSystemInfo> get_filesystem_info(GDBusObject* dbus_object)
{
    std::string_view const dbus_path = g_dbus_object_get_object_path(dbus_object);

    if (dbus_path.find(BLOCK_DEVICES_PATH) != 0)
    {
        return std::nullopt;
    }

    UDisksObject* udisks_object = UDISKS_OBJECT(dbus_object);

    UDisksBlock* block = udisks_object_peek_block(udisks_object);

    if (block == NULL)
    {
        return std::nullopt;
    }

    UDisksFilesystem* filesystem = udisks_object_peek_filesystem(udisks_object);
    if (filesystem == NULL)
    {
        return std::nullopt;
    }

    // Check if device is removable or hotpluggable
    std::string_view const device = udisks_block_get_device(block);
    std::string cmd = std::string("lsblk -n -o RM,HOTPLUG ") + std::string(device);
    std::string ret = getcommandoutput(std::move(cmd));

    gboolean mountable = false;
    if (ret.find("1") != std::string::npos)
    {
        mountable = true;
    }

    std::string_view const label = udisks_block_get_id_label(block);
    std::string_view const uuid = udisks_block_get_id_uuid(block);

    return FileSystemInfo
            {
                dbus_path,
                filesystem,
                label,
                uuid,
                mountable
            };
}


void log_filesystem_action(char const* const action, FileSystemInfo const& result)
{
    fprintf(stderr, "udisks2 block device %s: device = %s uuid = %s label = %s\n",
            action, result.dbus_path.substr(BLOCK_DEVICES_PATH.length()).data(),
            result.uuid.data(), result.label.data());
}


static void on_object_removed(GDBusObjectManager* manager, GDBusObject* dbus_object, gpointer user_data)
{
    std::optional<FileSystemInfo> opt_result = get_filesystem_info(dbus_object);

    if (!opt_result.has_value())
    {
        return;
    }

    auto & result = opt_result.value();
    log_filesystem_action("removed", result);
}


static void mount_and_add_filesystem(FileSystemInfo const & result)
{
    GVariantBuilder builder;

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "auth.no_user_interaction", g_variant_new_boolean(TRUE));
    // TODO mount read-only?
    GVariant* options = g_variant_builder_end(&builder);
    g_variant_ref_sink(options);

    gchar* mnt_path = NULL;
    GError* error = NULL;
    if (!udisks_filesystem_call_mount_sync(result.filesystem, options, &mnt_path, NULL, &error))
    {
        fprintf(stderr, "Failed to mount: %s\n", error->message);
        g_error_free(error);
    }
    else
    {
        fprintf(stderr, "Mounted at: %s\n", mnt_path);
        
        // Open a new Xfe window
        if (automount_open)
        {           
            std::string cmd_xfe = std::string("xfe ") + mnt_path + "&";            
            errno = 0;
            int ret = system(cmd_xfe.data());
            if (ret == -1)
            {
                fprintf(stderr, "%s\n", strerror(errno));
            }
        }

#ifdef HAVE_LIBNOTIFY
        send_notification(mnt_path);
#endif

        g_free(mnt_path);
    }
    g_variant_unref(options);
}

static void on_object_added(GDBusObjectManager*	manager, GDBusObject* dbus_object, gpointer user_data)
{
    std::optional<FileSystemInfo> opt_result = get_filesystem_info(dbus_object);

    if (opt_result.has_value())
    {
        auto & result = opt_result.value();
        log_filesystem_action("added", result);

        mount_and_add_filesystem(result);
    }
}

static void on_interface_added(GDBusObjectManager* manager, GDBusObject* dbus_object,
                               GDBusInterface* interface, gpointer user_data)
{
    on_object_added(manager, dbus_object, user_data);
}


void mount_unmounted_filesystems(GDBusObjectManager* manager)
{
    GList* const objects = g_dbus_object_manager_get_objects(manager);
    GList* current = objects;

    while (current != NULL)
    {
        GDBusObject* object = G_DBUS_OBJECT(current->data);

        std::optional<FileSystemInfo> opt_result = get_filesystem_info(object);
        if (opt_result.has_value())
        {
            auto & result = opt_result.value();

            gchar const* const* mountpoints = udisks_filesystem_get_mount_points(result.filesystem);
            if (mountpoints != NULL && *mountpoints == NULL && result.mountable)
            {
                // Found a mountable filesystem that is not mounted
                log_filesystem_action("found", result);
                mount_and_add_filesystem(result);
            }
        }

        g_object_unref(current->data);
        current = g_list_next(current);
    }
    g_list_free(objects);
}
     

// Return index of MTP name in vector or -1 if not found
int mtp_get_index(char* name)
{
    int ind = -1;
    
    for (unsigned int n = 0; n < MTP.size(); n++)
    {
        if (strncmp(MTP[n].name, name, strlen(name)) == 0)
        {
            ind = n;
            break;
        }
    }
   
    return ind;
}


// Monitor and mount MTP devices
gboolean mtp_monitor_mount(gpointer data)
{
    // Read mounts list
    std::string cmd = std::string("gio mount -li");
    std::string ret = getcommandoutput(cmd);
  
    // Initialize search position
    std::size_t pos = 0;

    // Reset MTP devices plugged flag
    for (unsigned int n = 0; n < MTP.size(); n++)
    {
        MTP[n].plugged = FALSE;
    }

    // Search for MTP devices
    while (1)
    {
        pos = ret.find(std::string("GProxyVolumeMonitorMTP"), pos);

        // GProxyVolumeMonitorMTP found at pos
        if (pos != std::string::npos)
        {
            // Search for MTP device name
            std::size_t pos2 = ret.find(std::string("activation_root=mtp://"), pos);
        
            // activation_root=mtp:// found
            if (pos2 != std::string::npos)
            {
                pos2 += 22;
                std::size_t pos3 = ret.find('/', pos2);
                
                // MTP device name
                std::string mtp_name = ret.substr (pos2, pos3 - pos2);

                // Index in MTP list
                int ind = mtp_get_index(mtp_name.data());

                // If new, add device to MTP list
                if (ind < 0)
                {
                    MTPDevice mtp_dev;                   
                    xf_strlcpy(mtp_dev.name, mtp_name.data(), strlen(mtp_name.data()) + 1);  // Safe string copy
                    mtp_dev.plugged = TRUE;          // Plugged
                    mtp_dev.to_automount = TRUE;     // To automount                   
                    MTP.push_back(mtp_dev);          // Add new device
                    ind = MTP.size() - 1;            // Index of new device
                }
                
                // If not new, get MTP device index
                else
                {
                    MTP[ind].plugged = TRUE;                // Plugged
                }
                
                // MTP gvfs path
                std::string mtp_path = std::string("/run/user/") + std::to_string(getuid()) + std::string("/gvfs/mtp:host=") + mtp_name;
                
                // If MTP device can be automounted
                if (MTP[ind].to_automount == 1)
                {
                    struct stat info;

                    // Not already mounted
                    if ( !(lstat(mtp_path.data(), &info) == 0) || !S_ISDIR(info.st_mode) )
                    {
                        std::string mtp_uri = std::string("mtp://") + mtp_name;

                        // Mount MTP device
                        cmd = std::string("gio mount ") + mtp_uri;
                        std::string ret_mnt = getcommandoutput(cmd);
                        if (ret_mnt != "")
                        {
                            fprintf(stderr, "Failed to mount: %s\n", ret_mnt.data());
                        }
                    }
                
                    // Maybe already mounted, check if directory not empty
                    else
                    {
                        // Count directory entries
                        DIR* dirp;
                        struct dirent* dp;
                        dirp = opendir(mtp_path.data());
                        int count = 0;
                        if (dirp)
                        {
                            while ((dp = readdir(dirp)) != NULL)
                            {                                
                                // Skip '.' and '..' entries
                                if ( (strncmp(dp->d_name, ".", 1) == 0) || (strncmp(dp->d_name, "..", 2) == 0) )
                                {
                                    continue;
                                }
                                else
                                {
                                    count++;
                                }
                            }
                        }

                        // Close directory
                        closedir(dirp);
                    
                        // Directory not empty, we are sure that MTP device is mounted
                        if (count > 0)
                        {
                            if (MTP[ind].to_automount == TRUE)
                            {
                                // Open a new Xfe window
                                if (automount_open)
                                {
                                    std::string cmd_xfe = std::string("xfe ") + mtp_path + "&";                                    
                                    errno = 0;
                                    int ret = system(cmd_xfe.data());
                                    if (ret == -1)
                                    {
                                        fprintf(stderr, "%s\n", strerror(errno));
                                    }
                                }
                            }

                            MTP[ind].to_automount = FALSE;  // Not to automount
                        }
                    }
                }                
            }
            
            // Increment search position
            pos++;
        }
        else
        {
            break;
        }
    }
    
    // Reset to_automount flag to 1 for unplugged devices
    for (unsigned int n = 0; n < MTP.size(); n++)
    {
        if (MTP[n].plugged == FALSE)
        {
            MTP[n].to_automount = TRUE;  // To automount
        }
    }
    
    return TRUE;
}


int main()
{
    // Get environment variables $HOME and $XDG_CONFIG_HOME
    std::string homedir;
    char* str = getenv("HOME");
    if (str != NULL)
    {
        homedir = str;        
    }
    else
    {
        homedir = ROOTDIR;        
    }
    std::string xdgconfighome;
    str = getenv("XDG_CONFIG_HOME");
    if (str != NULL)
    {
        xdgconfighome = str;        
    }
    else
    {
        xdgconfighome = homedir + PATHSEPSTRING CONFIGPATH;
    }
    
    // Read automount_open variable state from Xfe config file
    std::string cmd = std::string("grep automount_open ") + xdgconfighome + PATHSEPSTRING + "xfe/xferc";
    std::string ret = getcommandoutput(std::move(cmd));
    if (ret == "automount_open=1\n")
    {
        automount_open = true;
    }

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);

    GError* error = NULL;
    UDisksClient* client = udisks_client_new_sync(NULL, &error);

    if (client == NULL)
    {
        fprintf(stderr, "Error connecting to udisks daemon: %s\n", error->message);
        g_error_free(error);
        g_main_loop_unref(loop);
        return 1;
    }
    else
    {
        fprintf(stderr, "Running Xfe automounter...\n");
    }

    GDBusObjectManager* manager = udisks_client_get_object_manager(client);

    // Mount all mountable filesystems that are not mounted (does not handle MTP devices)
    mount_unmounted_filesystems(manager);

    g_signal_connect(manager, "object-added", G_CALLBACK(on_object_added), NULL);
    g_signal_connect(manager, "interface-added", G_CALLBACK(on_interface_added), NULL);
    g_signal_connect(manager, "object-removed", G_CALLBACK(on_object_removed), NULL);

    // Monitor and mount MTP devices
    g_timeout_add (MTP_CHECK_INTERVAL, mtp_monitor_mount, loop);

    // Main loop
    g_main_loop_run(loop);

    g_object_unref(client);
    g_main_loop_unref(loop);

    return 0;
}
