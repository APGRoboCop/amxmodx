// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// MySQL Module
//

#ifndef _INCLUDE_MYSQL_THREADING_H
#define _INCLUDE_MYSQL_THREADING_H

#include "IThreader.h"
#include "ISQLDriver.h"
#include <amtl/am-string.h>
#include <sh_stack.h>

struct QueuedResultInfo
{
	AmxQueryInfo amxinfo;
	bool connect_success;
	bool query_success;
	float queue_time;
};

class AtomicResult : 
	public IResultSet,
	public IResultRow
{
	friend class MysqlThread;
public:
	AtomicResult();
	~AtomicResult() override;
public:
	//free the handle if necessary (see IQuery).
	void FreeHandle() override;
	unsigned int RowCount() override;
	unsigned int FieldCount() override;
	const char *FieldNumToName(unsigned int num) override;
	bool FieldNameToNum(const char *name, unsigned int *columnId) override;
	bool IsDone() override;
	IResultRow *GetRow() override;
	void NextRow() override;
	void Rewind() override;
	bool NextResultSet() override;
public:
	const char *GetString(unsigned int columnId) override;
	virtual const char *GetStringSafe(unsigned int columnId);
	double GetDouble(unsigned int columnId) override;
	float GetFloat(unsigned int columnId) override;
	int GetInt(unsigned int columnId) override;
	bool IsNull(unsigned int columnId) override;
	const char *GetRaw(unsigned int columnId, size_t *length) override;
public:
	void CopyFrom(IResultSet *rs);
private:
	void _InternalClear();
private:
	unsigned int m_RowCount;
	unsigned int m_FieldCount;
	size_t m_AllocSize;
	ke::AString **m_Table;
	unsigned int m_CurRow;
	bool m_IsFree;
};

class MysqlThread : public IThread
{
public:
	MysqlThread();
	~MysqlThread() override;
public:
	void SetInfo(const char *host, const char *user, const char *pass, const char *db, int port, unsigned int max_timeout);
	void SetCharacterSet(const char *charset);
	void SetQuery(const char *query);
	void SetCellData(cell data[], ucell len);
	void SetForward(int forward);
	void Invalidate();
	void Execute();
public:
	void RunThread(IThreadHandle *pHandle) override;
	void OnTerminate(IThreadHandle *pHandle, bool cancel) override;
private:
	ke::AString m_query;
	ke::AString m_host;
	ke::AString m_user;
	ke::AString m_pass;
	ke::AString m_db;
	unsigned int m_max_timeout;
	ke::AString m_charset;
	int m_port;
	cell *m_data;
	ucell m_datalen;
	size_t m_maxdatalen;
	int m_fwd;
	QueuedResultInfo m_qrInfo;
	AtomicResult m_atomicResult;
};

#endif //_INCLUDE_MYSQL_THREADING_H

