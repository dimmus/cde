/*
 * CDE - Common Desktop Environment
 *
 * (c) Copyright 1993-2012 The Open Group
 * (c) Copyright 2012-2022 CDE Project contributors, see
 * CONTRIBUTORS for details
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include "vista.h"
#include "dbtype.h"
#include "dbswab.h"

#ifndef BASE_FILE_NAME
#define BASE_FILE_NAME "tiny"
#endif

#define DBD_MAP_SIZE 8

#define PAGE_SIZE		4096
#define FILE_TABLE_SIZE		10
#define RECORD_TABLE_SIZE	8
#define FIELD_TABLE_SIZE	49
#define SET_TABLE_SIZE		3
#define MEMBER_TABLE_SIZE	3
#define SORT_TABLE_SIZE		0
#define KEY_TABLE_SIZE		0

int main(void) {
    fwrite(dbd_VERSION, DBD_COMPAT_LEN, 1, stdout);

    INT dbd_map[DBD_MAP_SIZE] = {
	PAGE_SIZE,
	FILE_TABLE_SIZE,
	RECORD_TABLE_SIZE,
	FIELD_TABLE_SIZE,
	SET_TABLE_SIZE,
	MEMBER_TABLE_SIZE,
	SORT_TABLE_SIZE,
	KEY_TABLE_SIZE
    };

    for (int i = 0; i < DBD_MAP_SIZE; ++i) HTONS(dbd_map[i]);

    fwrite(dbd_map, sizeof(INT), DBD_MAP_SIZE, stdout);

    FILE_ENTRY file_entry[FILE_TABLE_SIZE] = {
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= DATA,
	    .ft_slots	= 0x20,
	    .ft_slsize	= 0x7E,
	    .ft_pgsize	= 0x1000,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= DATA,
	    .ft_slots	= 0x8,
	    .ft_slsize	= 0xFE,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= DATA,
	    .ft_slots	= 0x3C,
	    .ft_slsize	= 0x22,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= DATA,
	    .ft_slots	= 0x23,
	    .ft_slsize	= 0x3A,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= DATA,
	    .ft_slots	= 0x6,
	    .ft_slsize	= 0x9A,
	    .ft_pgsize	= 0x400,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= KEY,
	    .ft_slots	= 0x30,
	    .ft_slsize	= 0x2A,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= KEY,
	    .ft_slots	= 0x11,
	    .ft_slsize	= 0x74,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= KEY,
	    .ft_slots	= 0x4E,
	    .ft_slsize	= 0x1A,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= KEY,
	    .ft_slots	= 0x28,
	    .ft_slsize	= 0x32,
	    .ft_pgsize	= 0x800,
	    .ft_flags	= 0
	},
	{
	    .ft_desc	= 0,
	    .ft_status	= CLOSED,
	    .ft_type	= KEY,
	    .ft_slots	= 0x7,
	    .ft_slsize	= 0x90,
	    .ft_pgsize	= 0x400,
	    .ft_flags	= 0
	}
    };

    snprintf(file_entry[0].ft_name, FILENMLEN, "%s.d00", BASE_FILE_NAME);
    snprintf(file_entry[1].ft_name, FILENMLEN, "%s.d01", BASE_FILE_NAME);
    snprintf(file_entry[2].ft_name, FILENMLEN, "%s.d21", BASE_FILE_NAME);
    snprintf(file_entry[3].ft_name, FILENMLEN, "%s.d22", BASE_FILE_NAME);
    snprintf(file_entry[4].ft_name, FILENMLEN, "%s.d23", BASE_FILE_NAME);
    snprintf(file_entry[5].ft_name, FILENMLEN, "%s.k00", BASE_FILE_NAME);
    snprintf(file_entry[6].ft_name, FILENMLEN, "%s.k01", BASE_FILE_NAME);
    snprintf(file_entry[7].ft_name, FILENMLEN, "%s.k21", BASE_FILE_NAME);
    snprintf(file_entry[8].ft_name, FILENMLEN, "%s.k22", BASE_FILE_NAME);
    snprintf(file_entry[9].ft_name, FILENMLEN, "%s.k23", BASE_FILE_NAME);

    for (int i = 0; i < FILE_TABLE_SIZE; ++i) {
	FILE_ENTRY *p = &file_entry[i];

	HTONS(p->ft_desc);
	HTONS(p->ft_slots);
	HTONS(p->ft_slsize);
	HTONS(p->ft_pgsize);
	HTONS(p->ft_flags);
    }

    fwrite(file_entry, sizeof(FILE_ENTRY), FILE_TABLE_SIZE, stdout);

    RECORD_ENTRY record_entry[RECORD_TABLE_SIZE] = {
	{
	    .rt_file	= 0,
	    .rt_len	= 0x7E,
	    .rt_data	= 0x12,
	    .rt_fields	= 0,
	    .rt_fdtot	= 0x13,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0,
	    .rt_len	= 0x7E,
	    .rt_data	= 0x12,
	    .rt_fields	= 0x13,
	    .rt_fdtot	= 0x2,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0,
	    .rt_len	= 0x7E,
	    .rt_data	= 0x1E,
	    .rt_fields	= 0x15,
	    .rt_fdtot	= 0xC,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0,
	    .rt_len	= 0x7E,
	    .rt_data	= 0x13,
	    .rt_fields	= 0x21,
	    .rt_fdtot	= 0x2,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0x1,
	    .rt_len	= 0xFE,
	    .rt_data	= 0x12,
	    .rt_fields	= 0x23,
	    .rt_fdtot	= 0x2,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0x2,
	    .rt_len	= 0x22,
	    .rt_data	= 0x6,
	    .rt_fields	= 0x25,
	    .rt_fdtot	= 0x4,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0x3,
	    .rt_len	= 0x3A,
	    .rt_data	= 0x6,
	    .rt_fields	= 0x29,
	    .rt_fdtot	= 0x4,
	    .rt_flags	= 0
	},
	{
	    .rt_file	= 0x4,
	    .rt_len	= 0x9A,
	    .rt_data	= 0x6,
	    .rt_fields	= 0x2D,
	    .rt_fdtot	= 0x4,
	    .rt_flags	= 0
	}
    };

    for (int i = 0; i < RECORD_TABLE_SIZE; ++i) {
	RECORD_ENTRY *p = &record_entry[i];

	HTONS(p->rt_file);
	HTONS(p->rt_len);
	HTONS(p->rt_data);
	HTONS(p->rt_fields);
	HTONS(p->rt_fdtot);
	HTONS(p->rt_flags);
    }

    fwrite(record_entry, sizeof(RECORD_ENTRY), RECORD_TABLE_SIZE, stdout);

    FIELD_ENTRY field_entry[FIELD_TABLE_SIZE] = {
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x12,
	    .fd_rec	= 0,
	    .fd_flags	= 0x4
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x16,
	    .fd_rec	= 0,
	    .fd_flags	= 0x4
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x1A,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x1E,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x22,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x26,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x2A,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x2E,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x8,
	    .fd_dim	= {0x8, 0, 0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x32,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x32,
	    .fd_dim	= {0x32, 0, 0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x3A,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x6C,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x6E,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x70,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x72,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x74,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x76,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x78,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x7A,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x7C,
	    .fd_rec	= 0,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x12,
	    .fd_rec	= 0x1,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x6A,
	    .fd_dim	= {0x1, 0x6A, 0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x14,
	    .fd_rec	= 0x1,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x1E,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x4
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x22,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x4
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x26,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x2A,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x2E,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= UNIQUE,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x20,
	    .fd_dim	= {0x20, 0, 0},
	    .fd_keyfile	= 0x5,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x32,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x22,
	    .fd_dim	= {0x22, 0, 0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x52,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x74,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x76,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x78,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x7A,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x7C,
	    .fd_rec	= 0x2,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x13,
	    .fd_rec	= 0x3,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= DUPLICATES,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x69,
	    .fd_dim	= {0x1, 0x69, 0},
	    .fd_keyfile	= 0x6,
	    .fd_keyno	= 0x1,
	    .fd_ptr	= 0x15,
	    .fd_rec	= 0x3,
	    .fd_flags	= 0x400
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= SHORTINT,
	    .fd_len	= 0x2,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x12,
	    .fd_rec	= 0x4,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0xEA,
	    .fd_dim	= {0xEA, 0x1, 0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x14,
	    .fd_rec	= 0x4,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= UNIQUE,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x10,
	    .fd_dim	= {0x10, 0, 0},
	    .fd_keyfile	= 0x7,
	    .fd_keyno	= 0x2,
	    .fd_ptr	= 0x6,
	    .fd_rec	= 0x5,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x16,
	    .fd_rec	= 0x5,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x1A,
	    .fd_rec	= 0x5,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x1E,
	    .fd_rec	= 0x5,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= UNIQUE,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x28,
	    .fd_dim	= {0x28, 0, 0},
	    .fd_keyfile	= 0x8,
	    .fd_keyno	= 0x3,
	    .fd_ptr	= 0x6,
	    .fd_rec	= 0x6,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x2E,
	    .fd_rec	= 0x6,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x32,
	    .fd_rec	= 0x6,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x36,
	    .fd_rec	= 0x6,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= UNIQUE,
	    .fd_type	= CHARACTER,
	    .fd_len	= 0x86,
	    .fd_dim	= {0x86, 0, 0},
	    .fd_keyfile	= 0x9,
	    .fd_keyno	= 0x4,
	    .fd_ptr	= 0x6,
	    .fd_rec	= 0x7,
	    .fd_flags	= 0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x8E,
	    .fd_rec	= 0x7,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x92,
	    .fd_rec	= 0x7,
	    .fd_flags	= 0x0
	},
	{
	    .fd_key	= NOKEY,
	    .fd_type	= LONGINT,
	    .fd_len	= 0x4,
	    .fd_dim	= {0},
	    .fd_keyfile	= 0,
	    .fd_keyno	= 0,
	    .fd_ptr	= 0x96,
	    .fd_rec	= 0x7,
	    .fd_flags	= 0x0
	}
    };

    for (int i = 0; i < FIELD_TABLE_SIZE; ++i) {
	FIELD_ENTRY *p = &field_entry[i];

	HTONS(p->fd_len);
	HTONS(p->fd_keyfile);
	HTONS(p->fd_keyno);
	HTONS(p->fd_ptr);
	HTONS(p->fd_rec);
	HTONS(p->fd_flags);

	for (int j = 0; j < MAXDIMS; ++j) HTONS(p->fd_dim[j]);
    }

    fwrite(field_entry, sizeof(FIELD_ENTRY), FIELD_TABLE_SIZE, stdout);

    SET_ENTRY set_entry[SET_TABLE_SIZE] = {
	{
	    .st_order	= LAST,
	    .st_own_rt	= 0,
	    .st_own_ptr	= 0x6,
	    .st_members	= 0,
	    .st_memtot	= 0x1,
	    .st_flags	= 0
	},
	{
	    .st_order	= LAST,
	    .st_own_rt	= 0x2,
	    .st_own_ptr	= 0x6,
	    .st_members	= 0x1,
	    .st_memtot	= 0x1,
	    .st_flags	= 0
	},
	{
	    .st_order	= LAST,
	    .st_own_rt	= 0x2,
	    .st_own_ptr	= 0x12,
	    .st_members	= 0x2,
	    .st_memtot	= 0x1,
	    .st_flags	= 0
	}
    };

    for (int i = 0; i < SET_TABLE_SIZE; ++i) {
	SET_ENTRY *p = &set_entry[i];

	HTONS(p->st_order);
	HTONS(p->st_own_rt);
	HTONS(p->st_own_ptr);
	HTONS(p->st_members);
	HTONS(p->st_memtot);
	HTONS(p->st_flags);
    }

    fwrite(set_entry, sizeof(SET_ENTRY), SET_TABLE_SIZE, stdout);

    MEMBER_ENTRY member_entry[MEMBER_TABLE_SIZE] = {
	{
	    .mt_record		= 0x1,
	    .mt_mem_ptr		= 0x6,
	    .mt_sort_fld	= 0,
	    .mt_totsf		= 0
	},
	{
	    .mt_record		= 0x4,
	    .mt_mem_ptr		= 0x6,
	    .mt_sort_fld	= 0,
	    .mt_totsf		= 0
	},
	{
	    .mt_record		= 0x3,
	    .mt_mem_ptr		= 0x7,
	    .mt_sort_fld	= 0,
	    .mt_totsf		= 0
	}
    };

    for (int i = 0; i < MEMBER_TABLE_SIZE; ++i) {
	MEMBER_ENTRY *p = &member_entry[i];

	HTONS(p->mt_record);
	HTONS(p->mt_mem_ptr);
	HTONS(p->mt_sort_fld);
	HTONS(p->mt_totsf);
    }

    fwrite(member_entry, sizeof(MEMBER_ENTRY), MEMBER_TABLE_SIZE, stdout);

    fprintf(stdout,
	"OR_DBREC\n"
	"OR_DBMISCREC\n"
	"OR_OBJREC\n"
	"OR_MISCREC\n"
	"OR_BLOBREC\n"
	"OR_SWORDREC\n"
	"OR_LWORDREC\n"
	"OR_HWORDREC\n"
	"OR_DBFLAGS\n"
	"OR_DBUFLAGS\n"
	"OR_RECCOUNT\n"
	"OR_MAXDBA\n"
	"OR_AVAILD99\n"
	"OR_UNAVAILD99\n"
	"OR_HUFID\n"
	"OR_DBSECMASK\n"
	"OR_VERSION\n"
	"OR_DBFILL\n"
	"OR_DBOTYPE\n"
	"OR_COMPFLAGS\n"
	"OR_DBACCESS\n"
	"OR_MINWORDSZ\n"
	"OR_MAXWORDSZ\n"
	"OR_RECSLOTS\n"
	"OR_FZKEYSZ\n"
	"OR_ABSTRSZ\n"
	"OR_LANGUAGE\n"
	"OR_DBMISCTYPE\n"
	"OR_DBMISC\n"
	"OR_OBJFLAGS\n"
	"OR_OBJUFLAGS\n"
	"OR_OBJSIZE\n"
	"OR_OBJDATE\n"
	"OR_OBJSECMASK\n"
	"OR_OBJKEY\n"
	"OR_OBJFILL\n"
	"OR_OBJACCESS\n"
	"OR_OBJTYPE\n"
	"OR_OBJCOST\n"
	"OR_OBJHDROFFSET\n"
	"OR_OBJEUREKA\n"
	"OR_MISCTYPE\n"
	"OR_MISC\n"
	"OR_BLOBLEN\n"
	"OR_BLOB\n"
	"OR_SWORDKEY\n"
	"OR_SWOFFSET\n"
	"OR_SWFREE\n"
	"OR_SWADDRS\n"
	"OR_LWORDKEY\n"
	"OR_LWOFFSET\n"
	"OR_LWFREE\n"
	"OR_LWADDRS\n"
	"OR_HWORDKEY\n"
	"OR_HWOFFSET\n"
	"OR_HWFREE\n"
	"OR_HWADDRS\n"
	"OR_DB_MISCS\n"
	"OR_OBJ_BLOBS\n"
	"OR_OBJ_MISCS\n");

    return 0;
}
