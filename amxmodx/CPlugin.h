// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef PLUGIN_H
#define PLUGIN_H

#include "sh_list.h"
#include "amx.h"
#include "amxxfile.h"
#include <amtl/am-string.h>
#include <amtl/am-vector.h>
#include <amtl/am-autoptr.h>

// *****************************************************
// class CPluginMngr
// *****************************************************

enum
{
	ps_bad_load,	//Load failed
	ps_error,	//Erroneous state
	ps_locked,	//UNUSED
	ps_paused,	//Plugin is temporarily paused
	ps_stopped,	//Plugin is ... more temporarily paused
	ps_running,	//Plugin is running
};

struct AutoConfig
{
	ke::AString autocfg;
	ke::AString folder;
	bool create;
};

class CPluginMngr
{
public:

	class iterator;

	class CPlugin
	{
		friend class iterator;
		friend class CPluginMngr;
	
		AMX amx;
		void* code;
		
		ke::AString name;
		ke::AString version;
		ke::AString title;
		ke::AString author;
		ke::AString url;
		ke::AString description;
		ke::AString errorMsg;
		
		unsigned int failcounter;
		int m_PauseFwd;
		int m_UnpauseFwd;
		int paused_fun;
		int status;
		CPlugin* next;
		int id;
		
		CPlugin(int i, const char* p, const char* n, char* e, size_t m, int d);
		~CPlugin();
		
		bool m_Debug;
		cell* m_pNullStringOfs;
		cell* m_pNullVectorOfs;
		ke::Vector<ke::AutoPtr<AutoConfig>> m_configs;
	public:
		const char* getName() { return name.chars();}
		const char* getVersion() { return version.chars();}
		const char* getTitle() { return title.chars();}
		const char* getAuthor() { return author.chars();}
		const char* getUrl() { return url.chars(); }
		const char* getDescription() { return description.chars(); }
		const char* getError() { return errorMsg.chars();}
		int getStatusCode() { return status; }
		int getId() const { return id; }
		AMX* getAMX() { return &amx; }
		const AMX* getAMX() const { return &amx; }
		void setTitle(const char* n) { title = n; }
		void setAuthor(const char* n) { author =n; }
		void setVersion(const char* n) { version = n; }
		void setUrl(const char* n) { url = n; }
		void setDescription(const char* n) { description = n; }
		void setError(const char* n) { errorMsg = n; }
		bool isValid() const { return (status >= ps_paused); }
		bool isPaused() const { return ((status == ps_paused) || (status == ps_stopped)); }
		bool isStopped() const { return (status == ps_stopped); }
		bool isExecutable(int id) const { return (isValid() && !isPaused());	}
		
		void Finalize();
		void AddToFailCounter(unsigned int i);
		void pausePlugin();
		void unpausePlugin();
		void pauseFunction(int id);
		void unpauseFunction(int id);
		void setStatus(int a);
		
		const char* getStatus() const;
		bool isDebug() const { return m_Debug; }
		cell* getNullStringOfs() const { return m_pNullStringOfs; }
		cell* getNullVectorOfs() const { return m_pNullVectorOfs; }
	public:
		void AddConfig(bool create, const char *name, const char *folder);
		size_t GetConfigCount();
		AutoConfig *GetConfig(size_t index);
	}; 
	
private:	
	CPlugin *head;
	int pCounter;
public:
	CPluginMngr() { head = nullptr; pCounter = 0; pNatives = nullptr; m_Finalized=false;}
	~CPluginMngr() { clear(); InvalidateCache(); }

	bool m_Finalized;
	AMX_NATIVE_INFO *pNatives;

	// Interface

	CPlugin* loadPlugin(const char* path, const char* name, char* error, size_t maxLength, int debug);
	void unloadPlugin(CPlugin** a);
	int loadPluginsFromFile(const char* filename, bool warn=true);

	CPlugin* findPluginFast(AMX *amx) { return (CPlugin*)(amx->userdata[UD_FINDPLUGIN]); }
	CPlugin* findPlugin(AMX *amx);
	CPlugin* findPlugin(int index);
	CPlugin* findPlugin(const char* name);

	int getPluginsNum() const { return pCounter; }
	void Finalize();
	void clear();

	class iterator
	{
		CPlugin *a;
	public:
		iterator(CPlugin*aa) : a(aa) {}
		iterator& operator++() { a = a->next; return *this; }
		bool operator==(const iterator& b) const { return a == b.a; }
		bool operator!=(const iterator& b) const { return !operator==(b); }
		operator bool () const { return a ? true : false; }
		CPlugin& operator*() { return *a; }
	};

	iterator begin() const { return iterator(head); }
	iterator end() const { return iterator(nullptr); }
public:
	struct plcache_entry
	{
		CAmxxReader *file;
		size_t bufsize;
		char *buffer;
		ke::AString path;
	};
	char *ReadIntoOrFromCache(const char *file, size_t &bufsize);
	void InvalidateCache();
	void InvalidateFileInCache(const char *file, bool freebuf);
	void CacheAndLoadModules(const char *plugin);
	void CALMFromFile(const char *file);
private:
	List<plcache_entry *> m_plcache;
	List<ke::AString *> m_BlockList;
};

#endif //PLUGIN_H
