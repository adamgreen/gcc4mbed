/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_LOCALFILESYSTEM_H
#define MBED_LOCALFILESYSTEM_H

#include "platform/platform.h"

#if DEVICE_LOCALFILESYSTEM

#include "FileSystem.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/

/** A filesystem for accessing the local mbed Microcontroller USB disk drive
 *
 *  This allows programs to read and write files on the same disk drive that is used to program the
 *  mbed Microcontroller. Once created, the standard C file access functions are used to open,
 *  read and write files.
 *
 * @Note Synchronization level: Thread safe
 *
 * Example:
 * @code
 * #include "mbed.h"
 *
 * LocalFileSystem local("local");               // Create the local filesystem under the name "local"
 *
 * int main() {
 *     FILE *fp = fopen("/local/out.txt", "w");  // Open "out.txt" on the local file system for writing
 *     fprintf(fp, "Hello World!");
 *     fclose(fp);
 *     remove("/local/out.txt");                 // Removes the file "out.txt" from the local file system
 *
 *     DIR *d = opendir("/local");               // Opens the root directory of the local file system
 *     struct dirent *p;
 *     while((p = readdir(d)) != NULL) {         // Print the names of the files in the local file system
 *       printf("%s\n", p->d_name);              // to stdout.
 *     }
 *     closedir(d);
 * }
 * @endcode
 *
 * @note
 *  If the microcontroller program makes an access to the local drive, it will be marked as "removed"
 *  on the Host computer. This means it is no longer accessible from the Host Computer.
 *
 *  The drive will only re-appear when the microcontroller program exits. Note that if the program does
 *  not exit, you will need to hold down reset on the mbed Microcontroller to be able to see the drive again!
 */
class LocalFileSystem : public FileSystem
{
public:
    LocalFileSystem(const char* pName);

    /** Mounts a filesystem to a block device
     *
     *  @param bd       BlockDevice to mount to
     *  @return         0 on success, negative error code on failure
     */
    virtual int mount(BlockDevice *bd);

    /** Mounts a filesystem to a block device
     *
     *  @param bd       BlockDevice to mount to
     *  @param force    Flag to force the underlying filesystem to force mounting the filesystem
     *  @return         0 on success, negative error code on failure
     */
    virtual int mount(BlockDevice *bd, bool force);

    /** Unmounts a filesystem from the underlying block device
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual int unmount();

    /** Remove a file from the filesystem.
     *
     *  @param path     The name of the file to remove.
     *  @return         0 on success, negative error code on failure
     */
    virtual int remove(const char *path);

    /** Rename a file in the filesystem.
     *
     *  @param path     The name of the file to rename.
     *  @param newpath  The name to rename it to
     *  @return         0 on success, negative error code on failure
     */
    virtual int rename(const char *path, const char *newpath);

    /** Store information about the file in a stat structure
     *
     *  @param path     The name of the file to find information about
     *  @param st       The stat buffer to write to
     *  @return         0 on success, negative error code on failure
     */
    virtual int stat(const char *path, struct stat *st);

    /** Create a directory in the filesystem.
     *
     *  @param path     The name of the directory to create.
     *  @param mode     The permissions with which to create the directory
     *  @return         0 on success, negative error code on failure
     */
    virtual int mkdir(const char *path, mode_t mode);

protected:
    /** Open a file on the filesystem
     *
     *  @param file     Destination for the handle to a newly created file
     *  @param path     The name of the file to open
     *  @param flags    The flags to open the file in, one of O_RDONLY, O_WRONLY, O_RDWR,
     *                  bitwise or'd with one of O_CREAT, O_TRUNC, O_APPEND
     *  @return         0 on success, negative error code on failure
     */
    virtual int file_open(fs_file_t *file, const char *path, int flags);

    /** Close a file
     *
     *  @param file     File handle
     *  return          0 on success, negative error code on failure
     */
    virtual int file_close(fs_file_t file);

    /** Read the contents of a file into a buffer
     *
     *  @param file     File handle
     *  @param buffer   The buffer to read in to
     *  @param size     The number of bytes to read
     *  @return         The number of bytes read, 0 at end of file, negative error on failure
     */
    virtual ssize_t file_read(fs_file_t file, void *buffer, size_t len);

    /** Write the contents of a buffer to a file
     *
     *  @param file     File handle
     *  @param buffer   The buffer to write from
     *  @param size     The number of bytes to write
     *  @return         The number of bytes written, negative error on failure
     */
    virtual ssize_t file_write(fs_file_t file, const void *buffer, size_t len);

    /** Flush any buffers associated with the file
     *
     *  @param file     File handle
     *  @return         0 on success, negative error code on failure
     */
    virtual int file_sync(fs_file_t file);

    /** Move the file position to a given offset from from a given location
     *
     *  @param file     File handle
     *  @param offset   The offset from whence to move to
     *  @param whence   The start of where to seek
     *      SEEK_SET to start from beginning of file,
     *      SEEK_CUR to start from current position in file,
     *      SEEK_END to start from end of file
     *  @return         The new offset of the file
     */
    virtual off_t file_seek(fs_file_t file, off_t offset, int whence);

    /** Get the file position of the file
     *
     *  @param file     File handle
     *  @return         The current offset in the file
     */
    virtual off_t file_tell(fs_file_t file);

    /** Get the size of the file
     *
     *  @param file     File handle
     *  @return         Size of the file in bytes
     */
    virtual size_t file_size(fs_file_t file);

    /** Open a directory on the filesystem
     *
     *  @param dir      Destination for the handle to the directory
     *  @param path     Name of the directory to open
     *  @return         0 on success, negative error code on failure
     */
    virtual int dir_open(fs_dir_t *dir, const char *path);

    /** Close a directory
     *
     *  @param dir      Dir handle
     *  return          0 on success, negative error code on failure
     */
    virtual int dir_close(fs_dir_t dir);

    /** Read the next directory entry
     *
     *  @param dir      Dir handle
     *  @param ent      The directory entry to fill out
     *  @return         1 on reading a filename, 0 at end of directory, negative error on failure
     */
    virtual ssize_t dir_read(fs_dir_t dir, struct dirent *ent);

    /** Set the current position of the directory
     *
     *  @param dir      Dir handle
     *  @param offset   Offset of the location to seek to,
     *                  must be a value returned from dir_tell
     */
    virtual void dir_seek(fs_dir_t dir, off_t offset);

    /** Get the current position of the directory
     *
     *  @param dir      Dir handle
     *  @return         Position of the directory that can be passed to dir_rewind
     */
    virtual off_t dir_tell(fs_dir_t dir);

    /** Rewind the current position to the beginning of the directory
     *
     *  @param dir      Dir handle
     */
    virtual void dir_rewind(fs_dir_t dir);
};

} // namespace mbed

#endif
#endif

/** @}*/
