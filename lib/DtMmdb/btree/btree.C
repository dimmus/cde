/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
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
/*
 * $XConsortium: mmdb_btree.cc /main/3 1996/06/11 17:14:32 cde-hal $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
 * All rights reserved.  Unpublished -- rights reserved under
 * the Copyright Laws of the United States.  USE OF A COPYRIGHT
 * NOTICE IS PRECAUTIONARY ONLY AND DOES NOT IMPLY PUBLICATION
 * OR DISCLOSURE.
 * 
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE
 * SECRETS OF HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.  USE,
 * DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT THE
 * PRIOR EXPRESS WRITTEN PERMISSION OF HAL COMPUTER SYSTEMS
 * INTERNATIONAL, LTD.
 * 
 *                         RESTRICTED RIGHTS LEGEND
 * Use, duplication, or disclosure by the Government is subject
 * to the restrictions as set forth in subparagraph (c)(l)(ii)
 * of the Rights in Technical Data and Computer Software clause
 * at DFARS 252.227-7013.
 *
 *          HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.
 *                  1315 Dell Avenue
 *                  Campbell, CA  95008
 * 
 */


#include "btree/btree.h"


btree::btree(const char* store_name)
{
   int err;
   MDB_txn *txn;

   key_DBT.mv_data = 0;
   key_DBT.mv_size = 0;

   if ((err = mdb_env_create(&btree_env)))
      throw(stringException(mdb_strerror(err)));

   if ((err = mdb_env_open(btree_env, store_name, MDB_NOSUBDIR | MDB_NOSYNC,
                           0640)))
      throw(stringException(mdb_strerror(err)));

   txn = txn_begin();

   if ((err = mdb_dbi_open(txn, NULL, 0, &btree_DB))) {
      mdb_txn_abort(txn);
      throw(stringException(mdb_strerror(err)));
   }

   txn_commit(txn);
}

btree::~btree()
{
   int err;

   if ((err = mdb_env_sync(btree_env, 0))) {
      cerr << mdb_strerror(err);
      std::exit(1);
   }

   mdb_dbi_close(btree_env, btree_DB);
   mdb_env_close(btree_env);
}

void btree::clean()
{
   throw(stringException("void btree::clean(): not implemented yet"));
}

void btree::data_t_2_DBT(data_t& w)
{
   switch (w.flag) {
     case data_t::INT:
      key_DBT.mv_data = &w.key.int_key;
      key_DBT.mv_size = sizeof(w.key.int_key);
      break;

     case data_t::STRING:
      key_DBT.mv_data = w.key.str_key;
      key_DBT.mv_size = strlen(w.key.str_key);
      break;

     case data_t::VOID:
        throw(stringException("btree data_t_2_DBT: unknown key type"));
   }
}

Boolean btree::insert(data_t& w)
{
   int err;
   MDB_txn *txn;

   data_t_2_DBT(w);

   MDB_val data_DBT;
   data_DBT.mv_data = &w.dt;
   data_DBT.mv_size = sizeof(w.dt);

   txn = txn_begin();

   err = mdb_put(txn, btree_DB, &key_DBT, &data_DBT, 0);

   if (err && err != MDB_MAP_FULL && err != MDB_TXN_FULL) {
      mdb_txn_abort(txn);
      throw(stringException(mdb_strerror(err)));
   }

   txn_commit(txn);

   if (err) {
      cerr << mdb_strerror(err);
      return false;
   }

   return true;
}

Boolean btree::remove(data_t& w)
{
   int err;
   MDB_txn *txn;

   data_t_2_DBT(w);

   txn = txn_begin();

   if ((err = mdb_del(txn, btree_DB, &key_DBT, NULL))) {
      mdb_txn_abort(txn);
      throw(stringException(mdb_strerror(err)));
   }

   txn_commit(txn);

   return true;
}

Boolean btree::member(data_t& w)
{
   int err;
   MDB_txn *txn;

   data_t_2_DBT(w);
   MDB_val data_DBT;

   txn = txn_begin(MDB_RDONLY);

   err = mdb_get(txn, btree_DB, &key_DBT, &data_DBT);

   mdb_txn_abort(txn);

   if (err) {
      if (err != MDB_NOTFOUND) throw(stringException(mdb_strerror(err)));
      return false;
   }

   if (data_DBT.mv_size != sizeof(w.dt))
      throw(stringException("btree get: tree corrupted"));

   memcpy((char*)&w.dt, data_DBT.mv_data, data_DBT.mv_size);

   return true;
}

ostream& btree::asciiOut(ostream& out)
{
   return out;
}

istream& btree::asciiIn(istream& in)
{
   return in;
}

MDB_txn *btree::txn_begin(unsigned int flags) {
   int err;
   MDB_txn *txn;

   if ((err = mdb_txn_begin(btree_env, NULL, flags, &txn)))
      throw(stringException(mdb_strerror(err)));

   return txn;
}

void btree::txn_commit(MDB_txn *txn) {
   int err;

   if ((err = mdb_txn_commit(txn)))
      throw(stringException(mdb_strerror(err)));
}
