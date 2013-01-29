#include"stdafx.h"
#include"bwtraining.h"

#include<assert.h>
#include<io.h>
#include<direct.h> //mkdir
#include <mmsystem.h> //PlaySound

#include<algorithm> // sort

// FILE FORMAT:
// [MAIN]
// author=
// title=
// description=
// date=MMDDYYYY
// [BO1]
// MATCHUP=ZvT
// AT=02:00,build pool[120],*.wav
// [BO2]
// [COMMON]
// RACE=Zv*
// AT=10:00+02:00,make units,*.wav

#define BO_FILE "bwdata.txt"
#define BANK_FILE "bwcoach.cfg"

#define TAG_MAIN_AUTHOR "author"
#define TAG_MAIN_TITLE "title"
#define TAG_MAIN_DESC "description"
#define TAG_MAIN_DATE "date"
#define TAG_MAIN_APM "apm"
#define TAG_MAIN_PLAYER "player"
#define TAG_MAIN_TRFILE "trfile"
#define TAG_MATCHUP "matchup"
#define TAG_AT "at"

#define DEFAULT_SOUND "message.wav"

int TrGoal::DELTA_TIME=5; // 5 seconds

//------------------------------------------------------------------------------------------

static const char *_GetRootPath()
{
	static char root[255]="";
	if(root[0]==0)
	{
		GetModuleFileName(0,root,sizeof(root));
		char *p=strrchr(root,'\\'); if(p) *p=0;
		p=strrchr(root,'\\'); if(p && (_stricmp(p+1,"debug")==0 || _stricmp(p+1,"release")==0)) *p=0;
		strcat(root,"\\");
	}
	return root;
}

//------------------------------------------------------------------------------------------

char TrFile::gSoundsDir[255]="";
void TrFile::SetSoundsDir(const char *path) {strcpy(gSoundsDir,path);}
const char *TrFile::GetSoundsDir() 
{
	if(gSoundsDir[0]==0) 
		sprintf(gSoundsDir,"%s%s\\",_GetRootPath(),SOUNDS_DIR);
	if(gSoundsDir[strlen(gSoundsDir)-1]!='\\') strcat(gSoundsDir,"\\");
	return gSoundsDir;
}

//------------------------------------------------------------------------------------------

TrFile::TrFile(bool ignore, bool training) : m_currentBO(-1), m_ignore(ignore)
{
	m_subdir=training ? TRAINING_DIR : RESULTS_DIR;
}

TrFile::~TrFile() 
{
	for(int i=0;i<(int)m_common.size();i++) delete m_common[i];
	for(int i=0;i<(int)m_bos.size();i++) delete m_bos[i];
}

//------------------------------------------------------------------------------------------

// matchup summary
const char *TrFile::MatchUpSummary() const
{
	static char buffer[255];
	buffer[0]=0;
	// player bos
	for(int i=0;i<(int)m_bos.size();i++) 
	{
		TrGoalList *bo = m_bos[i];
		if(i>0) strcat(buffer,", ");
		strcat(buffer,bo->MatchUp().MuStr());
		if(strlen(buffer)>=249) break;
	}
	return buffer;
}

//------------------------------------------------------------------------------------------

int TrFile::Load(const char *folder, const char *file, int *lineNum, bool fullPath)
{
	m_currentSection="";
	m_currentGoalList=0;

	if(!fullPath)
	{
		// save file name
		m_folder = folder;
		m_filename = file;
		file = GetFullPath();
	}
	else
	{
		// extract file name from path
		m_filename = strrchr(file,'\\')+1;
	}

	// open training file
	FILE *fp=fopen(file,"rb");
	if(fp==0) return -30;

	// get file size
	fseek(fp,0,SEEK_END);
	unsigned long fsize=(unsigned long)ftell(fp);
	fseek(fp,0,SEEK_SET);

	// read line by line
	int err=0;
	char line[255];
	char section[128];
	char entry[255];
	section[0]=0;
	entry[0]=0;
	unsigned long sizeRead=0;
	*lineNum=0;
	bool newSection=false;
	bool isDossier=false;
	while(fgets(line,sizeof(line),fp)!=0 && err==0)
	{
		// update size read
		sizeRead+=strlen(line);
		unsigned long percentage = (100UL*sizeRead)/fsize;

		// remove end of line
		char *text=strtok(line,"\r\n");
		if(text==0) continue;
		(*lineNum)++;

		// skip blanks
		while(*text==' ') text++;

		// comment?
		if(text[0]==';' || text[0]==0) continue;

		// if it is a section
		if(text[0]=='[')
		{
			text++;
			char *p=strrchr(text,']');
			if(p!=0)
			{
				// new section
				*p=0; 
				strcpy(section,text);
				newSection=true;
				// dossier?
				isDossier = _stricmp(section,SECTION_DOSSIER)==0;
			}
			else
			{
				section[0]=0;
			}
		}
		else
		{
			// must be an entry, extract entry name
			char *data=0;
			if(isDossier)
			{
				// a dossier line has no "entry=value" format
				entry[0]=0;
				data = text;
			}
			else
			{
				// extract "entry = value"
				char *p=strtok(text,"=");
				if(p==0) continue;
				strcpy(entry,p);
				data=p+strlen(p)+1;
			}

			// process entry
			if(section[0]!=0 && data) 
			{
				err=_ProcessEntry(section, newSection, entry, data, (int)percentage);
				newSection=false;
			}
		}
	}

	// add last goal list
	if(m_currentGoalList!=0) _AddGoalList(m_currentGoalList);

	//close file
	fclose(fp);
	return err;
}

//------------------------------------------------------------------------------------------

void TrGoalList::Save(const char *filepath)
{
	FILE *fp = fopen(filepath,"ab");
	if(fp==0) return;

	// model bo
	CString buf;
	char szDuration[12];
	char szRepeat[12];
	fprintf(fp,"[%s]\r\n",Name());
	fprintf(fp,"%s=%sv%s\r\n",TAG_MATCHUP,m_mu.RaceStr(),m_mu.RaceOppStr());
	fprintf(fp,"%s=%s\r\n",TAG_MAIN_AUTHOR,Author());
	fprintf(fp,"%s=%s\r\n",TAG_MAIN_DESC,Desc());
	fprintf(fp,"%s=%s\r\n",TAG_MAIN_DATE,Date());
	for(int i=0;i<GetCount();i++)
	{
		const TrGoal *goal = GetGoal(i);

		// duration
		if(goal->Duration()>0) sprintf(szDuration,"[%d]",goal->Duration());
		else szDuration[0]=0;

		// repeat
		if(goal->Repeat()>0)sprintf(szRepeat,"+%02d:%02d",goal->Repeat()/60,goal->Repeat()%60);
		else szRepeat[0]=0;

		// description
		buf.Format("%02d:%02d%s,%s%s",goal->TimeBegin()/60,goal->TimeBegin()%60,szRepeat,goal->Message(),szDuration);
		fprintf(fp,"%s=%s",TAG_AT,(const char*)buf);
		
		// add sound
		if(goal->HasSound()) fprintf(fp,",%s",goal->Sound());

		// save to file
		fprintf(fp,"\r\n");
	}

	// close file
	fprintf(fp,"\r\n");
	fclose(fp);
}

//------------------------------------------------------------------------------------------

void TrFileHeader::Save(const char *filepath, int flag) const
{
	FILE *fp = fopen(filepath,"wb");
	if(fp==0) return;

	fprintf(fp,"[%s]\r\n",SECTION_MAIN);

	// [MAIN] section
	if(flag&TRAINING_DATA)
	{
		fprintf(fp,"%s=%s\r\n",TAG_MAIN_TITLE,Title());
		fprintf(fp,"%s=%s\r\n",TAG_MAIN_AUTHOR,Author());
		fprintf(fp,"%s=%s\r\n",TAG_MAIN_DESC,Desc());
	}

	if(flag&RESULTS_DATA)
	{
		fprintf(fp,"%s=%s\r\n",TAG_MAIN_PLAYER,Player());
		fprintf(fp,"%s=%s\r\n",TAG_MAIN_TRFILE,TrFile());
		fprintf(fp,"%s=%d\r\n",TAG_MAIN_APM,APM());
	}

	// save date in all cases
	fprintf(fp,"%s=%s\r\n\r\n",TAG_MAIN_DATE,Date());
	fclose(fp);
}

//------------------------------------------------------------------------------------------

// save to file
int TrFile::Save(const char *filepath) const
{
	int err=0;

	// [MAIN] section
	Header().Save(filepath,TrFileHeader::TRAINING_DATA);

	// common bos
	for(int i=0;i<(int)m_common.size();i++) 
	{
		TrGoalList *bo = (TrGoalList *)(m_common[i]);
		bo->Save(filepath);
	}

	// player bos
	for(int i=0;i<(int)m_bos.size();i++) 
	{
		TrGoalList *bo = (TrGoalList *)(m_bos[i]);
		bo->Save(filepath);
	}

	return err;
}

//------------------------------------------------------------------------------------------

// save to file
int TrFileResult::Save(int apm, TrGoalList *modelBO)
{
	// store final model BO and APM
	m_bos.push_back(modelBO);
	m_header.SetAPM(apm);
	m_header.SetDate(CTime::GetCurrentTime().Format("%d/%m/%Y"));

	int err=0;
	const char *filepath = GetFullPath(true);

	ASSERT(m_bos.size()>=2);

	// [MAIN] section
	Header().Save(filepath,TrFileHeader::RESULTS_DATA);

	// player bo
	GetPlayerBO()->Save(filepath);

	// model bo
	GetModelBO()->Save(filepath);

	return err;
}

//------------------------------------------------------------------------------------------

int TrFile::_ProcessEntry(const char *section, bool newSection, const char *entry, char *data, int percentage)
{
	// process entry
	if(_stricmp(section,SECTION_MAIN)==0)
		return m_header.ProcessEntryMain(entry, data);
	else if(_stricmp(section,SECTION_DOSSIER)==0)
		return _ProcessDossierLine(data);
	else
		return _ProcessEntryGoals(section, newSection, entry, data);

	// display progress bar
	// ??? to do

	return -1;
}

//------------------------------------------------------------------------------------------

int TrFile::_ProcessDossierLine(const char *data)
{
	if(!m_dossier.empty()) m_dossier+="\r\n";
	m_dossier += data;
	return 0;
}

//------------------------------------------------------------------------------------------

int TrFileHeader::ProcessEntryMain(const char *entry, const char *data)
{
	if(_stricmp(entry,TAG_MAIN_AUTHOR)==0)
		m_author = data;
	else if(_stricmp(entry,TAG_MAIN_TITLE)==0)
		m_title = data;
	else if(_stricmp(entry,TAG_MAIN_DESC)==0)
		m_description = data;
	else if(_stricmp(entry,TAG_MAIN_DATE)==0)
		m_date = data;
	else if(_stricmp(entry,TAG_MAIN_PLAYER)==0)
		m_player = data;
	else if(_stricmp(entry,TAG_MAIN_TRFILE)==0)
		m_trfile = data;
	else if(_stricmp(entry,TAG_MAIN_APM)==0)
		m_apm = atoi(data);
	else
		return -2;
	return 0;
}

//------------------------------------------------------------------------------------------

void TrFile::_AddGoalList(TrGoalList *list)
{
	if(_stricmp(list->Name(),SECTION_COMMON)==0)
		m_common.push_back(list);
	else
		m_bos.push_back(list);
}


//------------------------------------------------------------------------------------------

void TrFile::AddBO(TrGoalList *list)
{
	m_bos.push_back(list);
}

//------------------------------------------------------------------------------------------

int TrFile::_ProcessEntryGoals(const char *section, bool newSection, const char *entry, char *data)
{
	// did we change section?
	if(newSection)
	{
		m_currentSection = section;
		if(m_currentGoalList!=0) _AddGoalList(m_currentGoalList);
		m_currentGoalList = new TrGoalList(section);
	}

	assert(m_currentGoalList!=0);
	if(_stricmp(entry,TAG_MATCHUP)==0)
	{
		// find first letter & convert to lower case
		int race=0,opprace=0;
		while(*data==' ') data++;
		char cRace = tolower(data[0]);
		// convert race letter to race value
		if(cRace=='z') race = RACE_Z;
		else if(cRace=='t') race = RACE_T;
		else if(cRace=='p') race = RACE_P;
		else if(cRace=='x' || cRace=='*') race = RACE_ANY;
		else return -3;
		cRace = tolower(data[2]);
		// convert race letter to race value
		if(cRace=='z') opprace = RACE_Z;
		else if(cRace=='t') opprace = RACE_T;
		else if(cRace=='p') opprace = RACE_P;
		else if(cRace=='x' || cRace=='*') opprace = RACE_ANY;
		else return -3;
		// store race
		m_currentGoalList->SetRace(race);
		m_currentGoalList->SetOppRace(opprace);
	}
	else if(_stricmp(entry,TAG_AT)==0)
	{
		// extract time
		char *pTime=strtok(data,",");
		if(pTime==0) return -5;
		// extract message
		char *pMsg=strtok(0,",");
		if(pMsg==0) return -6;
		// extract wave (if any)
		char *pWav=strtok(0,",");
		// create new goal
		TrGoal *goal = new TrGoal();
		int err = goal->Extract(pTime,pMsg,pWav);
		if(err==0)
			m_currentGoalList->AddGoal(goal);
		else 
			return err;
	}
	else if(_stricmp(entry,TAG_MAIN_AUTHOR)==0)
		m_currentGoalList->SetAuthor(data);
	else if(_stricmp(entry,TAG_MAIN_DESC)==0)
		m_currentGoalList->SetDesc(data);
	else if(_stricmp(entry,TAG_MAIN_DATE)==0)
		m_currentGoalList->SetDate(data);
	else
		return -4;
	return 0;
}

//------------------------------------------------------------------------------------------

//ctor
TrGoal::TrGoal() : m_state(ST_IDLE), m_waitForKey(false), m_timeS(0), m_nextTimeS(0), m_repeatS(0), m_durationS(0), m_type(0) 
{
}

TrGoal::TrGoal(int times, int duration, const char *msg, const char *wav)
 : m_state(ST_IDLE), m_waitForKey(false), m_timeS(times), m_nextTimeS(times), m_repeatS(0), m_durationS(duration) 
{
	m_message=msg;   
	if(wav!=0) m_sound=wav; 
    m_type = _GetTypeFromMessage();
}

TrGoal::TrGoal(const TrGoal *model, int timeBegin, int delta)
{ 
	// properties
	m_type = model->m_type;
	m_timeS=		timeBegin!=-1 ? timeBegin : model->m_timeS;     
	m_message=		model->m_message;   
	m_sound=		model->m_sound;     
	m_repeatS=		delta==-1 ? model->m_repeatS : delta;
	m_durationS=	model->m_durationS; 
						         
	// working vables for evaluation
	m_nextTimeS=	model->m_nextTimeS; 
	m_waitForKey=	model->m_waitForKey;
	m_state=		model->m_state;     
}

//------------------------------------------------------------------------------------------

int TrGoal::_GetTypeFromMessage() const
{
	if(_strnicmp(m_message.c_str(),"add ",4)==0) return TYPE_BUILD;
	if(_strnicmp(m_message.c_str(),"build",5)==0) return TYPE_BUILD;
	if(_strnicmp(m_message.c_str(),"warp",4)==0) return TYPE_BUILD;
	if(_strnicmp(m_message.c_str(),"evolv",5)==0) return TYPE_BUILD;
	if(_strnicmp(m_message.c_str(),"train",5)==0) return TYPE_TRAIN;
	if(_strnicmp(m_message.c_str(),"hatch",5)==0) return TYPE_TRAIN;
	if(_strnicmp(m_message.c_str(),"morph",5)==0) return TYPE_TRAIN;
	if(_strnicmp(m_message.c_str(),"resea",5)==0) return TYPE_TECH;
	if(_strnicmp(m_message.c_str(),"upgra",5)==0) return TYPE_TECH;
	return TYPE_OTHER;
}

//------------------------------------------------------------------------------------------

int TrGoal::Extract(const char *time, const char *msg, const char *wav)
{
	// time (MM:SS[+MM:SS])
	if(strlen(time)!=5 && strlen(time)!=11)	return -6;

	// extract event time
	int m1=atoi(time);
	if(m1<0 || m1>59) return -7;
	int s1=atoi(time+3);
	if(s1<0 || s1>59) return -8;
	m_nextTimeS = m_timeS = m1*60+s1;

	// if more info
	if(strlen(time)==11)
	{
		// extract event repetition interval
		int m2=atoi(time+6);
		if(m2<0 || m2>59) return -9;
		int s2=atoi(time+9);
		if(s2<0 || s2>59) return -10;
		m_repeatS = m2*60+s2;
	}

	// extract message and duration (if any)
	char *pDuration = (char*)strchr(msg,'[');
	if(pDuration!=0) {*pDuration=0; pDuration++;}
	m_message = msg;
	if(pDuration!=0) 
	{
		// find duration from bocode
		m_durationS = atoi(pDuration);
		if(m_durationS==-1) return -11;
		if(m_durationS > 0) m_waitForKey=true;
	}

	// type
	m_type = _GetTypeFromMessage();

	// wav file name
	if(wav!=0) 
	{
		m_sound = wav;
		if(strchr(wav,'.')==0) m_sound+=".wav"; // add default extension

		// check existence
		if(_access(TrGoal::GetSoundPath(),0)!=0)
			m_sound = "";
	}
	return 0;
}

//------------------------------------------------------------------------------------------

void TrGoalList::AddGoal(TrGoal *goal)
{
	m_goals.push_back(goal);
}

//------------------------------------------------------------------------------------------

void TrGoalList::Evaluate(int currentTime, const TrMatchup& mu, TrGoal **goalToAdd)
{
	// check match up
	if(!m_mu.IsAny() && !(mu==m_mu)) return;

	for(int i=0; i<(int)m_goals.size(); i++)
	{
		if(m_goals[i]->Evaluate(currentTime))
		{
			*goalToAdd = m_goals[i];
			break;
		}
	}
}

//------------------------------------------------------------------------------------------

void TrGoalList::CompleteAll(int currentTime)
{
	for(int i=0; i<(int)m_goals.size(); i++)
	{
		if(m_goals[i]->TimeBegin()<currentTime)
			m_goals[i]->ForceComplete();
	}
}

//------------------------------------------------------------------------------------------

int TrGoalList::GetMaxTime() const
{
	int maxt=0;
	for(int i=0; i<(int)m_goals.size(); i++)
	{
		TrGoal *goal = m_goals[i];
		if(goal->TimeEnd()>=maxt) maxt=goal->TimeEnd();
	}
	return maxt;
}

//------------------------------------------------------------------------------------------

bool TrGoal::Evaluate(int currentTime)
{
	if(m_state==ST_IDLE && (currentTime+DELTA_TIME)>=m_nextTimeS)
	{
		m_state = m_waitForKey ? ST_WAITINGFORKEY : ST_COMPLETE;
		if(m_repeatS>0) {while(m_nextTimeS<=currentTime) m_nextTimeS += m_repeatS; m_state=ST_IDLE;}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------

void TrGoal::ForceComplete()
{
	m_state = ST_COMPLETE;
}

//------------------------------------------------------------------------------------------

void TrFile::Reset(const TrMatchup& mu, int currentTime) 
{
	// clear goal queue
	m_goalQueue.clear();

	// reset common bos
	for(int i=0;i<(int)m_common.size();i++) m_common[i]->Reset();

	// reset normal bos
	m_currentBO = -1;
	for(int i=0;i<(int)m_bos.size();i++) 
	{
		//reset goal list
		TrGoalList *bo = m_bos[i];
		bo->Reset();
		// find first bo suitable for current race
		if(bo->MatchUp()==mu && m_currentBO==-1) m_currentBO=i;
	}

	// if we are in the middle of a game or simulation
	if(m_currentBO!=-1 &&currentTime>0)
	{
		//  we need to disable all goals that are before the current time
		m_bos[m_currentBO]->CompleteAll(currentTime);
	}
}

//------------------------------------------------------------------------------------------

// move to next valid BO
bool TrFile::NextBO(const TrMatchup& mu, int currentTime)
{
	// try all bos
	int bo = m_currentBO; 
	for(int i=0;i<(int)m_bos.size();i++)
	{
		// move to next bo
		bo++;
		if(bo==m_bos.size()) bo=0;

		// right match up?
		if(m_bos[bo]->MatchUp()==mu && bo!=m_currentBO) 
		{
			// we found a new BO, we need to disable all goals that are
			// before the current time
			m_currentBO=bo;
			m_bos[m_currentBO]->CompleteAll(currentTime);
			return true;
		}
	}

	// no bo found for required race, keep same
	return false;
}

//------------------------------------------------------------------------------------------

// return the number of BOs for a specific matchup
int TrFile::GetMatchupCount(const TrMatchup& mu, int& currentBOIdx) const
{
	int count=0;
	for(int i=0;i<(int)m_bos.size();i++) 
	{
		TrGoalList *bo = m_bos[i];
		if(i==m_currentBO) currentBOIdx=count;
		if(bo->MatchUp()==mu) count++;
	}
	return count;
}

//------------------------------------------------------------------------------------------

// to call every second
bool TrFile::Evaluate(int currentTime, int playerRace, TrGoal **goalToAdd)
{
	if(m_ignore) return false;

	// evaluate common goals
	*goalToAdd = 0;
	for(int i=0;i<(int)m_common.size();i++) 
		m_common[i]->Evaluate(currentTime,playerRace,goalToAdd);

	// evaluate current BO
	TrGoalList *bo = GetCurrentBO();
	if(bo!=0) bo->Evaluate(currentTime,playerRace,goalToAdd);

	return *goalToAdd != 0;
}

//------------------------------------------------------------------------------------------

TrFileBank::TrFileBank() : m_selectedIdx(0)
{
}

//------------------------------------------------------------------------------------------

int TrFileBank::_LoadFile(const char *folder, const char *filename, bool ignore)
{				
	// load training file
	int lineNum=0;
	TrFile *file = new TrFile(ignore);
	int errf = file->Load(folder, filename, &lineNum);
	if(errf==-30) {delete file; return errf;}
	
	// add in bank
	push_back(file);

	// if there was an error during loading, set ignore flag
	if(errf!=0) file->SetIgnore(true);

	// show parsing error
	if(errf!=0 && errf!=-30)
	{
		// parsing error in training file
		CString msg;
		msg.Format("Syntax error %d in file /%s/%s at line %d",errf,folder,filename,lineNum);
		MessageBox(0,msg,"BWCoach",MB_ICONEXCLAMATION);
	}
	return errf;
}

//------------------------------------------------------------------------------------------

int TrFileBank::Load(const char *folder)
{							   
	int err=0;

	// save folder name
	m_folder = folder;

	// build path to bank file
	char path[255];
	if(strcmp(folder,".")!=0)
		sprintf(path,"%s"TRAINING_DIR"\\%s\\%s",_GetRootPath(),folder,BANK_FILE);
	else
		sprintf(path,"%s"TRAINING_DIR"\\%s",_GetRootPath(),BANK_FILE);
	m_inifile = path;

	// clear vector
	for(int i=0;i<(int)size();i++) delete (*this)[i];
	clear();

	// open bank file
	FILE *fp=fopen(m_inifile.c_str(),"rb");
	bool fileMissing=false;
	if(fp!=0) 
	{
		// read line by line
		char line[255];
		char entry[8];
		entry[0]=0;
		while(fgets(line,sizeof(line),fp)!=0 && err==0)
		{
			// remove end of line
			char *text=strtok(line,"\r\n");
			if(text==0) continue;

			// skip blanks
			while(*text==' ') text++;

			// comment?
			if(text[0]==';' || text[0]==0) continue;

			// extract "entry = value"
			char *p=strtok(text,"=");
			if(p==0) continue;
			strcpy(entry,p);
			char *data=p+strlen(p)+1;

			// process entry 
			int errf=_LoadFile(folder, data, atoi(entry)?true:false);
			if(errf==-30) fileMissing=true;
		}

		//close file
		fclose(fp);
	}

	// add files in training dir that are not yet in cfg file
	int fileAdded = _BrowseTrainingDir(folder);

	// update bank file?
	if(fileAdded>0 || fileMissing) Save();

	return err;
}

//------------------------------------------------------------------------------------------

// add files in training dir that are not yet in cfg file
int TrFileBank::_BrowseTrainingDir(const char *folder)
{
	// build path to training file dir
	char path[255];
	if(strcmp(folder,".")!=0)
		sprintf(path,"%s%s\\%s\\*"TRAINING_EXT,_GetRootPath(),TRAINING_DIR,folder);
	else
		sprintf(path,"%s%s\\*"TRAINING_EXT,_GetRootPath(),TRAINING_DIR);

	// list files
	CFileFind finder;
	BOOL bWorking = finder.FindFile(path);
	int fileAdded=0;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if(!_Exist(finder.GetFileName()))
			if(_LoadFile(folder, finder.GetFileName(), false)==0)
				fileAdded++;
	}

	return fileAdded;
}

//------------------------------------------------------------------------------------------

bool TrFileBank::_Exist(const char *filename) const
{
	for(int i=0;i<(int)size();i++)
	{
		TrFile *file = (*this)[i];
		if(_stricmp(file->FileName(),filename)==0) 
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------

int TrFileBank::Save()
{
	int err=0;

	// create file
	FILE *fp = fopen(m_inifile.c_str(),"wb");
	if(fp==0) return -31;

	// save all files
	for(int i=0;i<(int)size();i++)
	{
		TrFile *file = (*this)[i];
		fprintf(fp,"%d=%s\r\n",file->Ignore()?1:0,file->FileName());
	}

	// close file
	fclose(fp);
	return err;
}

//------------------------------------------------------------------------------------------

int TrFileBank::_GetNextFile(int currentidx)
{
	TrFile *file=0;
	int i=0;
	for(;i<(int)size();i++)
	{
		currentidx = (currentidx+1)%size();
		TrFile *file = (*this)[currentidx];
		if(!file->Ignore()) break;
	}
	return i==size() ? -1 : currentidx;
}

//------------------------------------------------------------------------------------------

// get current bo description
const char *TrFileBank::GetCurrentBODesc() const
{
	TrFile *file = GetTrainingFile();
	if(file==0) return "<no file>";
	TrGoalList *bo = file->GetCurrentBO();
	if(bo==0) return "<no BO for match up>";
	static char desc[255];
	int idx;
	int count = file->GetMatchupCount(m_mu,idx);
	sprintf(desc,"%d/%d %s",idx+1,count, bo->Name());
	return desc;
}

//------------------------------------------------------------------------------------------

// init training settings
void TrFileBank::InitTraining(int fileidx, const TrMatchup *mu) 
{
	if(size()==0) return;
	m_selectedIdx=fileidx; 
	if(mu) m_mu=*mu;
	TrFile *file = (*this)[m_selectedIdx];
	file->Reset(m_mu);
}

//------------------------------------------------------------------------------------------

// change race during simulation or game
void TrFileBank::UpdateRace(const TrMatchup& mu,int currentTime) 
{
	if(size()==0) return;
	m_mu=mu;
	TrFile *file = (*this)[m_selectedIdx];
	file->Reset(m_mu,currentTime);
}

//------------------------------------------------------------------------------------------

// next bo
bool TrFileBank::NextBO(int currentTime)
{
	if(size()==0) return false;
	TrFile *file = (*this)[m_selectedIdx];
	return file->NextBO(m_mu, currentTime);
}

//------------------------------------------------------------------------------------------

// next file
bool TrFileBank::NextFile()
{
	if(size()==0) return false;
	int idx = _GetNextFile(m_selectedIdx);
	if(idx!=-1)
	{
		UpdateSelectedFile(idx);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------

// update selected file
void TrFileBank::UpdateSelectedFile(int idx)
{
	m_selectedIdx = idx;
	TrFile *file = (*this)[m_selectedIdx];
	if(file!=0) file->Reset(m_mu);
}

//------------------------------------------------------------------------------------------

// next matchup
bool TrFileBank::NextMatchup(int currentTime, bool cycleBoth)
{
	if(!m_mu.Next(cycleBoth)) return false;
	UpdateRace(m_mu,currentTime);
	return true;
}

//------------------------------------------------------------------------------------------

const char *TrFile::GetDataPath(const char *subdir, bool forWriting)
{
	static char path[255];
	sprintf(path,"%s%s",_GetRootPath(),subdir);
	if(forWriting && _access(path,0)!=0)
		_mkdir(path);
	strcat(path,"\\");
	return path;
}

//------------------------------------------------------------------------------------------

// return full path
const char *TrFile::GetFullPath(bool forWriting) const
{
	// build path to training/result file
	char *path = (char *)GetDataPath(m_subdir.c_str(), forWriting);
	if(!m_folder.empty() && m_folder!=".") {strcat(path,m_folder.c_str()); strcat(path,"\\");}
	strcat(path,m_filename.c_str());
	return path;
}

//------------------------------------------------------------------------------------------

const char *TrGoal::GetSoundPath() const
{
	// build path to sound file
	static char path[255];
	sprintf(path,"%s%s",TrFile::GetSoundsDir(),m_sound.c_str());
	return path;
}

//------------------------------------------------------------------------------------------

const char *TrGoal::GetDefaultSoundPath() const
{
	// build path to sound file
	static char path[255];
	sprintf(path,"%s%s",TrFile::GetSoundsDir(),DEFAULT_SOUND);
	return path;
}

//------------------------------------------------------------------------------------------

// play sound for goal
void TrGoal::PlayGoalSound()
{
	// play sound
	if(HasSound())
		::PlaySound(GetSoundPath(),0,SND_FILENAME|SND_NOSTOP|SND_NOWAIT);
	else
		::PlaySound(GetDefaultSoundPath(),0,SND_FILENAME|SND_NOSTOP|SND_NOWAIT);
}

//------------------------------------------------------------------------------------------

TrGoal *TrFile::PopGoal() 
{
	if(GetQueueCount()>0) 
	{
		TrGoal *goal = m_goalQueue[0]; 
		goal->CompleteWithKey(); 
		m_goalQueue.erase(m_goalQueue.begin());
		return goal;
	}
	return 0;
}

//------------------------------------------------------------------------------------------

//ctor
TrFileResult::TrFileResult(const char *player, const TrFile* model, TrGoalList *playerBO) 
: TrFile(false,false) 
{
	// build file name
	CString fileName;
	CTime now = CTime::GetCurrentTime();
	fileName.Format("%s-%s%s",player,(const char*)now.Format("%y%m%d-%H%M"),RESULTS_EXT) ;

	m_header = model->Header();
	m_header.SetPlayer(player);
	m_header.SetTrFile(model->FileName());
	m_filename = (const char*)fileName;
	m_bos.push_back(playerBO);
}

//ctor
TrFileResult::TrFileResult(const char *filepath)
: TrFile(false,false)
{
	m_error = Load(0, filepath, &m_lineNum, true);
}

//------------------------------------------------------------------------------------------

