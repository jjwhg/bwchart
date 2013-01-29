#ifndef __BWTRAINING_H
#define __BWTRAINING_H

#include<assert.h>
#include<string>
#include<vector>

using namespace std;

#define RACE_ANY 0
#define RACE_Z 1
#define RACE_T 2
#define RACE_P 3

#define TRAINING_EXT ".trn"
#define RESULTS_EXT ".res"
#define TRAINING_DIR "training"
#define RESULTS_DIR "results"
#define SOUNDS_DIR "sounds"

#define SECTION_MAIN "main"
#define SECTION_COMMON "common"
#define SECTION_DOSSIER "strategy"

//------------------------------------------------------------------------------------------

class TrFileHeader
{
public:
	TrFileHeader() : m_apm(0) {}

	const TrFileHeader& operator =( const TrFileHeader& src )
	{
		m_author=		src.m_author;     
		m_title=		src.m_title;      
		m_description=	src.m_description;
		m_date=			src.m_date;       
		m_player=		src.m_player;     
		m_trfile=		src.m_trfile;     
		m_apm=			src.m_apm;     
		return *this;
	}

	// write value
	int ProcessEntryMain(const char *entry, const char *data);
	void SetAuthor(const char *author) {m_author=author;}
	void SetTitle(const char *title) {m_title=title;}
	void SetDesc(const char *desc) {m_description=desc;}
	void SetPlayer(const char *player) {m_player=player;}
	void SetTrFile(const char *trfile) {m_trfile=trfile;}
	void SetDate(const char *date) {m_date=date;}
	void SetAPM(int apm) {m_apm=apm;}

	// read value
	const char *Author() const {return m_author.c_str();}
	const char *Title() const {return m_title.c_str();}
	const char *Desc() const {return m_description.c_str();}
	const char *Date() const {return m_date.c_str();}
	const char *Player() const {return m_player.c_str();}
	const char *TrFile() const {return m_trfile.c_str();}
	int APM() const {return m_apm;}

	// save to file
	enum {TRAINING_DATA=1, RESULTS_DATA=2, ALL_DATA=3};
	void Save(const char *filepath, int flag) const;

private:
	// training file info
	string m_author;
	string m_title;
	string m_description;
	string m_date; // used also in results file

	// results file info
	string m_player;
	string m_trfile;
	int m_apm;
};

//------------------------------------------------------------------------------------------

class TrGoal
{
public:
	// ctors
	TrGoal();
	TrGoal(int times, int duration, const char *msg, const char *wav);
	TrGoal(const TrGoal *model, int timeBegin=-1, int delta=-1);

	// status
	enum {ST_IDLE=0, ST_WAITINGFORKEY, ST_COMPLETE, ST_COMPLETEWITHKEY};

	// type
	enum {TYPE_OTHER=0,TYPE_TRAIN, TYPE_TECH,TYPE_BUILD};

	// extract from file data
	int Extract(const char *time, const char *msg, const char *wav);

	// reset
	void Reset() {m_nextTimeS=m_timeS; m_state=ST_IDLE;}

	// evaluate
	bool Evaluate(int currentTime);

	// complete goal
	void CompleteWithKey() {m_state=ST_COMPLETEWITHKEY;}
	void ForceComplete();

	// play sound for goal
	void PlayGoalSound();

	// default sound
	const char *GetDefaultSoundPath() const;

	// accessors
	int Type() const {return m_type;}
	int TimeBegin() const {return m_timeS;}
	int TimeEnd() const {return m_timeS+m_durationS;}
	const char *Message() const {return m_message.c_str();}
	const char *Sound() const {return m_sound.c_str();}
	int Duration() const {return m_durationS;}
	int Repeat() const {return m_repeatS;}
	bool HasSound() const {return !m_sound.empty();}
	const char *GetSoundPath() const;
	const char *MessageNoDuration() const 
	{
		static char tmp[255];
		strcpy(tmp,m_message.c_str());
		char *p=strchr(tmp,'['); if(p!=0) *p=0;
		return tmp;
	}
	const char *MessageWithDelta() const 
	{
		static char tmp[255];
		strcpy(tmp,m_message.c_str());
		char *p=strchr(tmp,'['); if(p!=0) *p=0;
		if(m_repeatS!=0) sprintf(&tmp[strlen(tmp)]," (%s%d)",m_repeatS>0?"+":"-",m_repeatS);
		return tmp;
	}

	// change delta time
	static void SetDeltaTime(int delta) {DELTA_TIME=delta;}

private:
	// properties
	int m_type;
	int m_timeS;
	string m_message;
	string m_sound;
	int m_repeatS;
	int m_durationS;

	// working variables for evaluation
	int m_nextTimeS;
	bool m_waitForKey;
	int m_state;
	static int DELTA_TIME;

	int _GetTypeFromMessage() const;
};

typedef std::vector<TrGoal*> TrGoalArray;

//------------------------------------------------------------------------------------------

class TrMatchup
{
public:
	TrMatchup(int prace=RACE_ANY, int orace=RACE_ANY) : m_playerRace(prace), m_opponentRace(orace) {}
	int m_playerRace;
	int m_opponentRace;

	// player race
	const char *RaceStr() const {return m_playerRace==RACE_Z?"Z":m_playerRace==RACE_T?"T":m_playerRace==RACE_P?"P":"*";}
	// opponent race
	const char *RaceOppStr() const {return m_opponentRace==RACE_Z?"Z":m_opponentRace==RACE_T?"T":m_opponentRace==RACE_P?"P":"*";}
	// matchup description
	const char *MuStr() const 
	{
		static char szMu[4];
		sprintf(szMu,"%sv%s",RaceStr(),RaceOppStr());
		return szMu;
	}
	// undefined matchup?
	bool IsAny() const
	{
		return(m_playerRace==RACE_ANY && m_opponentRace==RACE_ANY);
	}
	// comparison
	bool operator == (const TrMatchup& mu) const
	{
		if(m_playerRace!=mu.m_playerRace) return false;
		if(m_opponentRace==RACE_ANY || mu.m_opponentRace==RACE_ANY) return true;
		return (m_opponentRace==mu.m_opponentRace);
	}
	// next matchup
	bool Next(bool cycleBoth=false)
	{
		if(m_playerRace==RACE_ANY) {if(!cycleBoth) return false; else m_playerRace=RACE_Z;}
		m_opponentRace++;
		if(m_opponentRace==4) 
		{
			m_opponentRace=RACE_Z;
			if(cycleBoth) {m_playerRace++; if(m_playerRace==4) m_playerRace=RACE_Z;}
		}
		return true;
	}
};

//------------------------------------------------------------------------------------------

class TrGoalList
{
public:
	TrGoalList(const char *section) : m_priority(0) {m_name=section;m_author="<unknown>";m_description="<no desc>";}
	TrGoalList(const TrGoalList* model)
	{
		m_priority=model->m_priority;
		m_name = model->m_name;
		m_mu = model->m_mu;
		m_author	 =model->m_author;     
		m_description=model->m_description;
		m_date		 =model->m_date;       
		for(int i=0;i<model->GetCount();i++) m_goals.push_back(new TrGoal(model->GetGoal(i)));
	}
	~TrGoalList() {for(int i=0;i<(int)m_goals.size();i++) delete m_goals[i];}

	const TrGoalList& operator =( const TrGoalList& src )
	{
		assert(0);     
		return *this;
	}

	// accessors
	int Priority() const {return m_priority;}
	const char *Name() const {return m_name.c_str();}
	const TrMatchup& MatchUp() const {return m_mu;}

	// read value
	const char *Author() const {return m_author.c_str();}
	const char *Desc() const {return m_description.c_str();}
	const char *Date() const {return m_date.c_str();}

	// write value
	void SetAuthor(const char *author) {m_author=author;}
	void SetDesc(const char *desc) {m_description=desc;}
	void SetDate(const char *date) {m_date=date;}

	// creation
	void SetRace(int race) {m_mu.m_playerRace = race;}
	void SetOppRace(int race) {m_mu.m_opponentRace = race;}
	void SetName(const char *name) {m_name = name;}
	void AddGoal(TrGoal *goal);
	void Reset() {for(int i=0;i<(int)m_goals.size();i++) m_goals[i]->Reset();}

	// evaluation
	void Evaluate(int currentTime, const TrMatchup& mu, TrGoal **goalToAdd);
	void CompleteAll(int currentTime);

	// _access to goals
	int GetCount() const {return m_goals.size();}
	const TrGoal *GetGoal(int i) const {return m_goals[i];}
	int GetMaxTime() const;

	// save to file
	void Save(const char *filepath);

private:
	int m_priority;
	// bo name
	string m_name;
	// match up
	TrMatchup m_mu;
	// all goals in that BO
	TrGoalArray m_goals;
	// bo info
	string m_author;
	string m_description;
	string m_date; // used also in results file
};

typedef std::vector<TrGoalList*> TrGoalListArray;

//------------------------------------------------------------------------------------------

class TrFile
{
public:
	//ctor
	TrFile(bool ignore, bool training=true);
	~TrFile();

	// load from file
	int Load(const char *folder, const char *file, int *lineNum, bool fullPath=false);

	// save to file
	void AddBO(TrGoalList *list);
	int Save(const char *filepath) const;

	// reset before starting a game
	void Reset(const TrMatchup& mu, int currentTime=0);

	// evaluate goals during game
	bool Evaluate(int currentTime, int playerRace, TrGoal **goalToAdd);

	// ignore flag
	bool Ignore() const {return m_ignore;}
	void SetIgnore(bool val) {m_ignore=val;}

	// header
	const TrFileHeader& Header() const {return m_header;}
	TrFileHeader* pHeader() {return &m_header;}

	// dossier
	const char *Dossier() const {return m_dossier.c_str();}

	// matchup summary
	const char *MatchUpSummary() const;

	// file name
	const char *FileName() const {return m_filename.c_str();}

	// return full path
	const char *GetFullPath(bool forWriting=false) const;

	// get current goal list (can be null)
	TrGoalList *GetCurrentBO() const {return m_currentBO==-1 ? 0 : m_bos[m_currentBO];}

	// get model goal list (the one used as a model during game)
	TrGoalList *GetModelBO() const {return m_bos[1];}

	// get player goal list (the one recorded during game)
	TrGoalList *GetPlayerBO() const {return m_bos[0];}

	// move to next valid BO (if any)
	bool NextBO(const TrMatchup& mu, int currentTime);

	// _access to goal lists
	int GetMatchupCount(const TrMatchup& mu, int& currentBOIdx) const; 
	int GetCount() const {return m_bos.size();}
	const TrGoalList *GetGoalList(int i) const {return m_bos[i];}

	// _access to goal queue
	void PushGoal(TrGoal *goal) {m_goalQueue.push_back(goal);}
	TrGoal *PopGoal();
	int GetQueueCount() const {return m_goalQueue.size();}
	TrGoal *GetQueueGoal(int idx) {return m_goalQueue[idx];}

	// get path to data files
	static const char *GetDataPath(const char *subdir, bool forWriting=false);

	//sounds directory
	static char gSoundsDir[];
	static void SetSoundsDir(const char *path);
	static const char *GetSoundsDir();

protected:
	//properties
	string m_dossier;
	TrFileHeader m_header;
	TrGoalListArray m_common;
	TrGoalListArray m_bos;

	// working variables
	string m_subdir;
	string m_filename;
	string m_folder;
	int m_currentBO;
	TrGoalArray m_goalQueue;
	bool m_ignore;

	// loading 
	string m_currentSection;
	TrGoalList *m_currentGoalList;

	int _ProcessEntry(const char *section, bool newSection, const char *entry, char *data, int percentage);
	int _ProcessEntryGoals(const char *section, bool newSection, const char *entry, char *data);
	int _ProcessDossierLine(const char *data);
	void _AddGoalList(TrGoalList *list);

};

typedef std::vector<TrFile*> TrFileArray;

//------------------------------------------------------------------------------------------

class TrFileResult : public TrFile
{
private:
	int m_lineNum;
	int m_error;

public:
	//ctor
	TrFileResult(const char *player, const TrFile* model, TrGoalList *playerBO);
	TrFileResult(const char *filepath);

	// accessor
	int Error() const {return m_error;}
	int LineNum() const {return m_lineNum;}

	// add result goal
	void AddGoal(TrGoal *goal) {if(GetPlayerBO()!=0) GetPlayerBO()->AddGoal(goal);}

	// save to file
	int Save(int apm, TrGoalList *modelBO);
};

typedef std::vector<TrFileResult*> TrFileResultArray;

//------------------------------------------------------------------------------------------

class TrFileBank : public TrFileArray
{
private:
	string m_folder;
	string m_inifile;
	int m_selectedIdx;
	TrMatchup m_mu;

	int _LoadFile(const char *folder, const char *filename, bool ignore);
	bool _Exist(const char *file) const;
	int _BrowseTrainingDir(const char *folder);
	int _GetNextFile(int currentidx);

public:
	TrFileBank();
	~TrFileBank() {for(int i=0;i<(int)size();i++) delete (*this)[i];}

	// load from config file and training dir
	int Load(const char *folder);
	// save config file
	int Save();
	// sort in priority order
	void Sort();

	// update selected file
	void UpdateSelectedFile(int idx);

	// init training settings
	void InitTraining(int fileidx, const TrMatchup *mu); 

	// change race during simulation or game
	void UpdateRace(const TrMatchup& mu,int currentTime);

	// next bo
	bool NextBO(int currentTime);

	// next file
	bool NextFile();

	// next matchup
	bool NextMatchup(int currentTime, bool cycleBoth=false);

	// get current bo description
	const char *GetCurrentBODesc() const;

	// get current training file
	TrFile *GetTrainingFile() const {return size()>0 ? (*this)[m_selectedIdx] : 0;}
	int GetSelectedIdx() const {return m_selectedIdx;}

	// get match up
	TrMatchup& MatchUp() {return m_mu;}
};

//------------------------------------------------------------------------------------------

#endif