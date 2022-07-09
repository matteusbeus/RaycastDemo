#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw_md.h"
#include "cdfh.h"

//----------------------------------------------------------------------
// SegaCD File Handler - by Chilly Willy
//   Inspired by the MikMod READER structure.
//----------------------------------------------------------------------

static uint8_t cd_Eof(CDFileHandle_t *handle)
{
    if (!handle)
        return 1;

    if (handle->pos >= handle->length)
        return 1;

    return 0;
}

static int32_t cd_Read(CDFileHandle_t *handle, void *ptr, int32_t size)
{
    int32_t pos, blk, len, read = 0;
    uint8_t *dst = ptr;

    if (!handle)
        return 0;

    while (size != 0)
    {
        if (handle->Eof(handle))
            return read;

        pos = handle->pos;
        blk = (pos >> 11) + handle->offset;
        if (handle->block != blk)
        {
            read_cd(blk, 1, (void *)0x6800);
            handle->block = blk;
        }

        len = 0x800 - (pos & 0x7FF);
        if (len > size)
            len = size;
        if (len > (handle->length - pos))
            len = (handle->length - pos);

        memcpy(dst, (char *)0x6800 + (pos & 0x7FF), len);

        handle->pos += len;
        dst += len;
        read += len;
        size -= len;
    }

    return read;
}

static uint8_t cd_Get(CDFileHandle_t *handle)
{
    int32_t pos, blk;

    if (handle->Eof(handle))
        return 0;

    pos = handle->pos;
    blk = (pos >> 11) + handle->offset;
    if (handle->block != blk)
    {
        read_cd(blk, 1, (void *)0x6800);
        handle->block = blk;
    }

    handle->pos++;
    return ((uint8_t*)0x6800)[pos & 0x7FF];
}

static int32_t cd_Seek(CDFileHandle_t *handle, int32_t offset, int32_t whence)
{
    int32_t pos;

    if (!handle)
        return -1;

    pos = handle->pos;
    switch(whence)
    {
        case SEEK_CUR:
            pos += offset;
            break;
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_END:
            pos = handle->length - offset - 1;
            break;
    }
    if (pos < 0)
        handle->pos = 0;
    else if (pos > handle->length)
        handle->pos = handle->length;
    else
        handle->pos = pos;

    return handle->pos;
}

static int32_t cd_Tell(CDFileHandle_t *handle)
{
    return handle ? handle->pos : 0;
}


CDFileHandle_t *cd_handle_from_offset(int32_t offset, int32_t length)
{
    CDFileHandle_t *handle = (CDFileHandle_t*)malloc(sizeof(CDFileHandle_t));
    if (handle)
    {
        handle->Eof  = &cd_Eof;
        handle->Read = &cd_Read;
        handle->Get  = &cd_Get;
        handle->Seek = &cd_Seek;
        handle->Tell = &cd_Tell;
        handle->offset = offset;
        handle->length = length;
        handle->block = -1; // nothing read yet
        handle->pos = 0;
    }
    return handle;
}

CDFileHandle_t *cd_handle_from_name(char *name)
{
    int32_t i;
    char temp[256];

    CDFileHandle_t *handle = (CDFileHandle_t*)malloc(sizeof(CDFileHandle_t));
    if (handle)
    {
        handle->Eof  = &cd_Eof;
        handle->Read = &cd_Read;
        handle->Get  = &cd_Get;
        handle->Seek = &cd_Seek;
        handle->Tell = &cd_Tell;

        i = strlen(name);
        while (i && (name[i] != '/'))
            i--;
        if (name[i] == '/')
        {
            if (i)
            {
                strncpy(temp, name, i);
                temp[i] = 0;
            }
            else
            {
                strcpy(temp, "/");
            }
            if (set_cwd(temp) < 0)
            {
                // error setting working directory
                free(handle);
                return NULL;
            }
            strncpy(temp, &name[i+1], 255);
            temp[255] = 0;
        }
        else
        {
            strncpy(temp, name, 255);
            temp[255] = 0;
        }

        if (find_dir_entry(temp) < 0)
        {
            // error finding entry
            free(handle);
            return NULL;
        }

        handle->offset = global_vars->DENTRY_OFFSET;
        handle->length = global_vars->DENTRY_LENGTH;
        handle->block = -1; // nothing read yet
        handle->pos = 0;
    }
    return handle;
}

void delete_cd_handle(CDFileHandle_t *handle)
{
    if (handle)
        free(handle);
}

