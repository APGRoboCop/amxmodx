// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef CRANK_H
#define CRANK_H

#define RANK_VERSION 11

#include "stdafx.h"
#include "amxxmodule.h"

// *****************************************************
// class Stats
// *****************************************************

struct Stats {
	int hits;
	int shots;
	int damage;
	int hs;
	int tks;
	int kills;
	int deaths;
	int bodyHits[9]; ////////////////////
	
	// SiDLuke start
	int bPlants;
	int bExplosions;
	int bDefusions;
	int bDefused;
	// SiDLuke end :D

	Stats();
	void commit(Stats* a);
};

// *****************************************************
// class RankSystem
// *****************************************************

class RankSystem
{
public:
	class RankStats;
	friend class RankStats;
	class iterator;

	class RankStats : public Stats {
		friend class RankSystem;
		friend class iterator;
		RankSystem*	parent;
		RankStats*	next;
		RankStats*	prev;
		char*		unique;
		short int	uniquelen;
		char*		name;
		short int	namelen;
		int			score;
		int			id;
		RankStats( const char* uu, const char* nn,  RankSystem* pp );
		~RankStats();
		void setUnique( const char* nn  );
		inline void goDown() {++id;}
		inline void goUp() {--id;}
		inline void addStats(Stats* a) { commit( a ); }
	public:
		void setName( const char* nn  );
		inline const char* getName() const { return name ? name : ""; }
		inline const char* getUnique() const { return unique ? unique : ""; }
		inline int getPosition() const { return id; }
		inline int updatePosition( Stats* points ) {
			return parent->updatePos( this , points );
		}
		inline void MarkToDelete() {
			this->score = (1<<31);
		}
	};

private:
	RankStats* head;
	RankStats* tail;
	int rankNum;

	struct scoreCalc{
		AMX amx;
		void* code;
		int func;
		cell amxAddr1;
		cell amxAddr2;
		cell *physAddr1;
		cell *physAddr2;
	} calc;

	void put_before( RankStats* a, RankStats* ptr );
	void put_after( RankStats* a, RankStats* ptr );
	void unlink( RankStats* ptr );
	int updatePos( RankStats* r ,  Stats* s );
	
public:

	RankSystem();
	~RankSystem();

	void saveRank( const char* filename );
	bool loadRank( const char* filename );
	RankStats* findEntryInRank(const char* unique, const char* name );
	RankStats* findEntryInRankByUnique(const char* unique);
	RankStats* findEntryInRankByPos(int position);
	//bool loadCalc(const char* filename, char* error);
	inline int getRankNum( ) const { return rankNum; }
	void clear();
	//void unloadCalc();

	class iterator {
		RankStats* ptr;
	public:
		iterator(RankStats* a): ptr(a){}
		inline iterator& operator--() { ptr = ptr->prev; return *this;}
		inline iterator& operator++() {	ptr = ptr->next; return *this; }
		inline RankStats& operator*() {	return *ptr;}
		operator bool () { return (ptr != nullptr); }
	};

	inline iterator front() {  return iterator(head);  }
	inline iterator begin() {  return iterator(tail);  }
};


#endif
