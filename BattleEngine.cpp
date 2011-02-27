#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Engine.h"
#include "Str.h"
#include "StrBuilder.h"
#include "XML.h"



long long HPRandom;

bool FileIsDebug;


// Save ID
//  Used for preventing loading of incompadible save files
#define SID						(long)8



long ExpNeededPerLevel;
long SidesOnAHitDie;


////////////////////////////////////////
// Function prototypes

// Make a new character at level 1
void MakeNewCharacter();

// Display the main menu
int MainMenu();



////////////////////////////////////////
// Variables

#ifdef DEBUG
	bool IsDebug = true;
#else
	bool IsDebug = false;
#endif

long long EnemyHP;
long NextEnemyToBeat;
long FirstEnemyToBeat;


long EnemyNumber;
long CharsPutted;
char LastCharInputed;


long CTH; // Chance to hit
long enemiesCTH;
long enemiesDmgDealt;

long HPGained;
long LevelsGained;

int  TempWeaponDice;
long TempWeaponSidesPerDie;
long TempWeaponDmgBonus;
long TempWeaponStrBonus;
long TempWeaponHitPercent;

long TempWeaponMinDmg;
int  WeaponMinDmg;

long TempWeaponMaxDmg;
int  WeaponMaxDmg;

long TempWeaponTotalDmg;
int  WeaponTotalDmg;

long TempDice;
long TempSides;
long TempBase;
long TempHit;

int  TempArmorClass;
long ArmorClassDiff;

FILE* save_file;
char fName[256];





////////////////////////////////////////
// Hero's Stats

char Alias[256];
long Level;
long long Exp;
long long ExpTNL;

long long CurrentHP;
long long MaxHP;
long long StartingHP;

char WeaponName[256];
long WeaponDice;
long WeaponSidesPerDie;
long WeaponDmgBonus;
long WeaponHitPercent;

char ArmorName[256];
long ArmorClass;


#define STAT_STR 0
#define STAT_DEX 1
#define STAT_CON 2
#define STAT_INT 3
#define STAT_WIS 4
#define STAT_CHA 5

long StatsLen;
long* Stats; // Stats = (ActualStats / 10) + StatBonuses
long* ActualStats;
long* StatBonuses;
char** StatNames;

long classID;

long TimesSaved;
long TimesDied;




int TotalEnemies;

class Enemy {
public:
	char Name[256];
	char Weapon[256];
	char Armor[256];

	long MinHP;
	long MaxHP;

	long MinDmg;
	long MaxDmg;

	long WeaponHitPercent;
	long ArmorClass;

	long long Exp;
	long Level;

	long ItemsDropped;
	Str* Items;
	Str* ItemChances;
};

Enemy** enemies;


class CharacterClass {
public:
	char name[256];
	long* statGainRate;
	long* initialStats;
};

CharacterClass** cc;





void Sleep(int millis)
{
	usleep(1000 * millis);
}

// Sleep if (DebugMode != true)
void Wait(int millis)
{
	if (!IsDebug)
		Sleep(millis<1?1:millis);
}



// Get HP gained at a specific level
long long GetHP()
{
	long long holdholdRandom = holdRandom;
	long long holdHPRandom = HPRandom;
	RandomSeed(HPRandom);

	long long hp = StartingHP;
	for (int i = 1; i < Level; i++)
		hp += 5 + Random()%SidesOnAHitDie + Stats[STAT_CON];

	HPRandom = holdHPRandom;
	RandomSeed(holdholdRandom);

	return hp;
}




const char* GetSummaryStats(bool WithColors, bool WithMobs)
{
	static StrBuilder sumBuf(4000);
	sumBuf.empty();
	FirstEnemyToBeat = (NextEnemyToBeat > 9) ? NextEnemyToBeat - 9 : 0;

	if (WithColors)
		sumBuf += "|T|D";

	sumBuf += StrPrint("%s\r\n\r\n", Alias);

	if (WithColors)
		sumBuf += "|O";

	long lvlPercent = (long)(100 * (((double)Exp) / ((double)ExpTNL)));
	if (lvlPercent > 9)
		sumBuf += StrPrint("Level:  %i.%i", Level, lvlPercent);
	else
		sumBuf += StrPrint("Level:  %i.0%i", Level, lvlPercent);

	if (IsDebug)
		sumBuf += StrPrint("  (Exp: %x/%x)", Exp, ExpTNL);

	sumBuf += StrPrint(
		"\nHP:     %l/%l\r\n"
		"Weapon: %s\r\n"
		"Damage: %id%i",
		MaxHP, MaxHP, WeaponName,
		WeaponDice, WeaponSidesPerDie);

	if (WeaponDmgBonus + Stats[STAT_STR] > 0)
		sumBuf += StrPrint(" + %i", WeaponDmgBonus + Stats[STAT_STR]);
	if (WeaponDmgBonus + Stats[STAT_STR] < 0)
		sumBuf += StrPrint(" - %i", -WeaponDmgBonus - Stats[STAT_STR]);

	sumBuf += StrPrint("\r\n"
		"Armor:  %s\r\n"
		"AC:     %i\r\n",
		ArmorName, ArmorClass);

	if (WithMobs)
	{
		if (WithColors)
			sumBuf += "|R";
		sumBuf += "\r\n\r\nPlease select an enemy to battle:";

		if (WithColors)
			sumBuf += "|W";
		sumBuf += "\r\n";

		unsigned long Padding = 0;
		for (int i = FirstEnemyToBeat; i < NextEnemyToBeat; i++)
		{
			unsigned long j = strlen(enemies[i]->Name);
			if (Padding < j)
				Padding = j;
		}
		Padding++;

		for (int i = FirstEnemyToBeat; i < NextEnemyToBeat; i++)
		{
			sumBuf += StrPrint("%i: ", i + 1 - FirstEnemyToBeat);
			if (WithColors)
				sumBuf += "|G";
			sumBuf += enemies[i]->Name;
			for (unsigned long j = 0; j < Padding - strlen(enemies[i]->Name); j++)
				sumBuf += " ";
			if (WithColors)
				sumBuf += "|B";
			sumBuf += StrPrint("(Lvl %i)", enemies[i]->Level);
			if (WithColors)
				sumBuf += "|W";
			sumBuf += "\r\n";
		}
	}
	return sumBuf;
}

void Save()
{
	FILE* f;
	const char* buf;
	CanQuit = false;

	TimesSaved++;

	f = fopen(fName, "w");
	fwrite(SID, f);
	fputs("\r\n\r\n", f);
	fputs(GetSummaryStats(false, true), f);
	for (int i = 0; i < 4; i++)
		fputs("\r\n", f);

	fputc(26, f); // Ctrl + Z
	fputc(IsDebug, f);
	fwrite(Level, f);
	fwrite(Exp, f);
	fwrite(ExpTNL, f);
	fwrite(HPRandom, f);
	fputs(WeaponName, f); fputs("\r\n", f);
	fwrite(WeaponDice, f);
	fwrite(WeaponSidesPerDie, f);
	fwrite(WeaponDmgBonus, f);
	fwrite(WeaponHitPercent, f);
	fputs(ArmorName, f); fputs("\r\n", f);
	fwrite(ArmorClass, f);
	fwrite(NextEnemyToBeat, f);
	for (int i = 0; i < StatsLen; i++)
		fwrite(Stats[i], f);
	fwrite(TimesSaved, f);
	fwrite(TimesDied, f);
	fwrite(holdRandom, f);

	fclose(f);

	CanQuit = true;
}


long Load(bool getName)
{
	FILE* f;
	strcpy(fName, StrPrint("%s.sav", Alias));
	
	f = fopen(fName, "r");
	if (!f)
	{
		if (getName)
		{
			ClearScreen();
			MakeNewCharacter();
			return 0;
		}
		return 1;
	}

	long FileSID;
	fread(&FileSID, f);

	if (FileSID != SID)
	{
		fclose(f);
		Print("|W|DError loading file.\n|T|DReason: |Y|DIncompadible SID.\n\n"
			"|WPress any key to continue.");
		PressAnyKey();
		return 1;
	}

	int nls = 0;
	while (true)
	{
		char cur, last;
		last = cur;
		cur = fgetc(f);
		if (cur == '\n')
		{
			if (last == '\r')
			{
				if (++nls == 5)
					break;
				last = cur;
				cur = fgetc(f);
			}
		}
		else
			nls = 0;
	}


	fgetc(f);
	IsDebug = fgetc(f);
	fread(&Level, f);
	fread(&Exp, f);
	fread(&ExpTNL, f);
	fread(&HPRandom, f);
	fgets(WeaponName, 256, f);
	fread(&WeaponDice, f);
	fread(&WeaponSidesPerDie, f);
	fread(&WeaponDmgBonus, f);
	fread(&WeaponHitPercent, f);
	fgets(ArmorName, 256, f);
	fread(&ArmorClass, f);
	fread(&NextEnemyToBeat, f);
	for (int i = 0; i < StatsLen; i++)
		fread(&Stats[i], f);
	fread(&TimesSaved, f);
	fread(&TimesDied, f);
	fread(&holdRandom, f);
	fclose(f);

	MaxHP = GetHP();
	CurrentHP = MaxHP;

	Save();

	return 0;
}


int PreviewLevel(const char* fName)
{
	FILE* f = fopen(StrPrint("%s.sav", fName), "r");
	if (!f)
		return -2;


	long FileSID;
	fread(&FileSID, f);

	if (FileSID != SID)
	{
		fclose(f);
		return -3;
	}

	int nls = 0;
	while (true)
	{
		char cur, last;
		last = cur;
		cur = fgetc(f);
		if (cur == '\n')
		{
			if (last == '\r')
			{
				if (++nls == 5)
					break;
				last = cur;
				cur = fgetc(f);
			}
		}
		else
			nls = 0;
	}

	char fAlias[256];
	fgetc(f);
	fgets(fAlias, 256, f);

	char fPass[256];
	fgets(fPass, 256, f);

	bool b;
	b = fgetc(f);
	long tmpLevel;
	fread(&tmpLevel, f);

	fclose(f);

	return tmpLevel;
}


void Die()
{
	Print("The %s hits you with |R%i point(s)|W of damage!\n"
			"\n"
			"Your HP is: |B0|W\n"
			"%s's HP: |B%l|W\n\n"
			"\n"
			"|YYou are dead!|W\n", enemies[EnemyNumber]->Name, enemiesDmgDealt, enemies[EnemyNumber]->Name, EnemyHP);
	Sleep(1000);

	Print("\nPress any key to continue . . .");
	PressAnyKey();

	ClearScreen();

	Wait(1500);
	//if (IsDebug)
	{
		//Print("You would have gotten a concussion if you weren't in DEBUG mode!\n");
		Print("You would have gotten a concussion if I finished coding this!\n");
		TimesDied++;
		Save();

		Sleep(2000);

		Print("Press any key to continue...\n");
		PressAnyKey();

		ClearScreen();
	}
	/*
	else
	{
		long LvlLoss = Level / 10 + 1;
		if (LvlLoss >= Level)
			LvlLoss = Level - 1;

		Level -= LvlLoss;

		ExpTNL = 0;
		for (int i = 1; i <= Level; i++)
			ExpTNL += Level * ExpNeededPerLevel;

		Exp = 0;
		MaxHP = GetHP(Level);
		Stats[STAT_STR] = Level/GAIN_STR_INTERVAL;
		TimesDied++;

		Save();

		if (LvlLoss > 0)
			Print("The concussion causes you to lose %i levels of Experience!\n\n"
				"Don't die :(\n", LvlLoss);
		else
			Print("The concussion causes you to lose all your Experience!\n\n"
				"Don't die :(\n");

		Sleep(1500);

		Print("\nPress any key to continue...\n");
		PressAnyKey();

		ClearScreen();
	}
	*/
}





char* loadDataBuffer;
int loadDataBufferLength;
int loadDataBufferPos = 0;

char loadNextChar()
{
	if (loadDataBufferPos++ < loadDataBufferLength)
		return loadDataBuffer[loadDataBufferPos];
	return -1;
}



void LoadData()
{
	// Check for 'BattleEngine.xml'
	FILE* f = fopen("BattleEngine.xml", "r");
	if (!f)
	{
		Print("Failed to open 'BattleEngine.xml'"
			  "\n\nPress any key to quit");
		PressAnyKey();
		Quit();
	}
	fclose(f);

/* SIZE CHECK
	unsigned int fSize = GetFileSize();
	if (fSize == 0)
	{
		Print("BattleEngine.xml has no data"
			"\n\nPress any key to quit");
		PressAnyKey();
		Quit();
	}
*/

/* COMPRESSION
	char* oldData = new char[fSize];
	for (int i = 0; i < fSize; i++)
		oldData[i] = fgetc(f); // ^ 255;
	CloseFile();

	char* data = new char[fSize*5];
	unsigned int size;
	if (tinf_gzip_uncompress(data, &size, oldData, fSize) == TINF_DATA_ERROR)
	{
		Print("ERRORRRR!!!");
		Quit();
	}

	delete oldData;


	if (size == 0)
	{
		Print("BattleEngine.xml has no data"
			"\n\nPress any key to quit");
		PressAnyKey();
		Quit();
	}


	// Parse file
	loadDataBufferLength = size;
	loadDataBuffer = data;

	XMLDocument* doc = ParseXML(loadNextChar);
	delete data;
*/

	// Parse file
	document doc;
	doc.parse("BattleEngine.xml", false);

	ClearScreen();

	Print("\n\n|T|DDone parsing XML...\n");
	Sleep(500);

/*
	int n;
	XMLElement*  rootData;
	XMLElement*  rootDataMobs;
	XMLElement** rootDataMobsMob;
	XMLElement*  rootDataRules;
	XMLElement*  rootDataRulesConsts;

	rootData = doc->getChildren(&n)[0];

	rootDataMobs = rootData->getChild("Mobs", &n)[0];
	rootDataMobsMob = rootDataMobs->getChild("Mob", &n);
	TotalEnemies = n;

	rootDataRules = rootData->getChild("Rules", &n)[0];
	rootDataRulesConsts = rootDataRules->getChild("Consts", &n)[0];



	// Load gameplay constants
	SidesOnAHitDie    = rootDataRulesConsts->getChild("Sides_On_A_Hit_Die", &n)[0]->getIntValue();
	ExpNeededPerLevel = rootDataRulesConsts->getChild("Experiance_Factor", &n)[0]->getIntValue();
	StartingHP        = rootDataRulesConsts->getChild("Starting_HP", &n)[0]->getIntValue();


	// Initialize mobs
	enemies = new Enemy*[TotalEnemies];
	for (int i = 0; i < TotalEnemies; i++)
		enemies[i] = new Enemy;

	// Load mobs
	for (int i = 0; i < TotalEnemies; i++)
	{

		#define GetExtXML(s) \
			 m->getAttribute(s, &n)[0]->getValue()

		#define GetA2IExtXML(s) \
			 m->getAttribute(s, &n)[0]->getIntValue()

		Enemy* en = enemies[i];
		XMLElement* m = rootDataMobsMob[i];

		// Get mob's stats
		strcpy(en->Name,              GetExtXML("Name"));
		strcpy(en->Weapon,            GetExtXML("Weapon"));
		strcpy(en->Armor,             GetExtXML("Armor"));
		       en->MinHP            = GetA2IExtXML("HPMin");
		       en->MaxHP            = GetA2IExtXML("HPMax");
		       en->MinDmg           = GetA2IExtXML("DmgMin");
		       en->MaxDmg           = GetA2IExtXML("DmgMax");
		       en->WeaponHitPercent = GetA2IExtXML("HitPercent");
		       en->ArmorClass       = GetA2IExtXML("AC");
		       en->Exp              = GetA2IExtXML("Exp");
		       en->Level            = GetA2IExtXML("Level");

		en->ItemsDropped = 0;
*/

/*
		// Check for items
		if (GetElement(((tmp = base) += "ItemDrops")) != null)
		{
			// Initialize items
			int items = atoi(GetExtXML("ItemsDrops"));
			en->ItemsDropped = items;
			en->Items        = new Str[items];
			en->ItemChances  = new Str[items];

			// Load items
			for (int i = 0; i < items; i++)
			{
				Str itemStr = "DropItem";
				itemStr += (long)(i+1);
				strcpy(en->Items[i],       GetExtXML(itemStr));

				itemStr = "DropChance";
				itemStr += (long)(i+1);
				strcpy(en->ItemChances[i], GetExtXML(itemStr));
			}
		}
		else
			en->ItemsDropped = 0;
*/
/*
	}
*/

	// delete doc;
}

void HandleArgs(int argc, char* argv[])
{
	if (argc == 1)
		return;

	Print("\n\nargv[1] = '%s'\n", argv[1]);

	// Help
	if (!strcmp(argv[1], "/?") || !strcmp(argv[1], "-?") ||
		!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
	{
		ClearScreen();

		Print("|T|DUsage: |WBattleEngine [save-file]\n"
			"       BattleEngine --hack [password]\n"
			"\n"
			"|YOptions:\n|W|D"
			"  -h --help            Print this message\n"
			"     --hack            Extract all data from BattleEngine.xml\n"
			"  -e --extract <file>  Extract all data from a .bes file\n");
		Quit();
	}

	#define EndsWith(s) \
		!strcmp(&(argv[1][strlen(argv[1]) - strlen(s)]), s)

/*
	// Make a new data file
	if (EndsWith("BattleEngine.xml"))
	{
		Print("\n    BattleEngine.xml -> Compressor -> BattleEngine.dat\n");
		OpenReadFile(argv[1], OPEN_EXISTING);
		InflateFile
		lzo_uint fSize = GetFileSize();
		lzo_bytep data = new lzo_byte[fSize];
		for (int i = 0; i < fSize; i++)
			data[i] = 255 ^ fgetc(f);
		CloseFile();

		lzo_uint newSize;
		lzo_bytep newData = new lzo_byte[fSize*3];
		lzo_voidp wrkmem = new char[LZO1X_1_MEM_COMPRESS];
		lzo1x_1_compress(data, fSize, newData, &newSize, wrkmem);

		OpenWriteFile("BattleEngine.dat", CREATE_ALWAYS);
		for (int i = 0; i < newSize; i++)
			WriteChar(newData[i]);
		CloseFile();

		delete data;
		delete newData;

		Quit();
	}
*/

/* EDITING DATA FILE
	// Extract the data file
	if (EndsWith("BattleEngine.dat"))
	{
		InflateFile(argv[1], StrPrint("%s.xml", argv[1]));
		Quit();
	}
*/

	if (EndsWith(".bes"))
	{
		strcpy(Alias, argv[1]);
		return;
	}


	Print("Press any key to continue . . .\n");

	PressAnyKey();

	return;
}

void MakeNewCharacter()
{
	FILE* f = fopen(fName, "r");
	if (f)
	{
		fclose(f);
		Print("There is already a character with that name!");
		Sleep(2000);
		Quit();
	}


	ClearScreen();

	IsDebug = false;

	Level = 1;
	Exp = 0;
	ExpTNL = ExpNeededPerLevel;
	MaxHP = GetHP();
	CurrentHP = MaxHP;

	strcpy(WeaponName, "wooden sword");
	WeaponDice = 1;
	WeaponSidesPerDie = 2;
	WeaponDmgBonus = 0;
	WeaponHitPercent = 80;

	for (int i = 0; i < StatsLen; i++)
	{
		ActualStats[i] = cc[classID]->initialStats[i];
		StatBonuses[i] = 0;
		Stats[i] = ActualStats[i]/10;
	}

	strcpy(ArmorName, "rags");
	ArmorClass = 1;

	NextEnemyToBeat = 1;
	TimesSaved = 0;
	RandomSeed(GetMillisecondCount());

	Save();
}

int MainMenu()
{
	DIR* dir;
	struct dirent* entry;
	int nFiles = 0;
	char fNames[1024][256];
	int fLvl[1024];

	dir = opendir(".");
	while (entry = readdir(dir))
	{
		int len = strlen(entry->d_name);
		if (len > 4 && !strcmp(&(entry->d_name[len - 4]), ".sav"))
		{
			char* name = fNames[nFiles++];
			strcpy(name, entry->d_name);
			name[len - 4] = 0;
		}
	}
	closedir(dir);

	for (int i = 0; i < nFiles; i++)
		fLvl[i] = PreviewLevel(fNames[i]);

//	Sleep(300);

	CanQuit = true;

	ClearScreen();

	strcpy(Alias, "");


	Print("|WYou step into the local saloon.\n"
		"The barkeeper asks you, \"Wut's 'ur name son?\"\n\n");

	if (nFiles > 0)
	{
		Print("|P\n Already existing characters:\n\n");
		unsigned long Padding = 0;
		for (int i = 0; i < nFiles; i++)
		{
			unsigned long j = strlen(fNames[i]);
			if (Padding < j)
				Padding = j;
		}
		Padding++;

		for (int i = 0; i < nFiles; i++)
		{
			if (!strcmp(fNames[i], "Quit") ||
				!strcmp(fNames[i], "quit") ||
				!strcmp(fNames[i], "Exit") ||
				!strcmp(fNames[i], "exit") ||
				!strcmp(fNames[i], "Q") ||
				!strcmp(fNames[i], "q"))
			{
				fLvl[i] = -911;
			}
			if (fLvl[i] > 0)
				Print("|G");
			else
				Print("|R");
			Print("%s", fNames[i]);
			for (unsigned long j = 0; j < Padding - strlen(fNames[i]); j++)
				Print(" ");
			if (fLvl[i] > 0)
				Print("|B(Lvl %i)\n", fLvl[i]);
			else if (fLvl[i] == -911)
				Print("|B(Invalid name)\n", fLvl[i]);
			else
				Print("|B(Error loading file)\n", fLvl[i]);
		}
	}
	Print("|Y|D\n --> |W");
	int i = 0;

	while (true)
	{
		LastCharInputed = GetChar();
/*
		PINPUT_RECORD ConInpRec;
		while (ConInpRec = GetInput())
		{
			if ((ConInpRec->EventType == KEY_EVENT) &&
				 ConInpRec->Event.KeyEvent.bKeyDown)
				if (LastCharInputed = (unsigned char)ConInpRec->Event.KeyEvent.uChar.AsciiChar)
					break;
		}
*/
		if ('A' <= LastCharInputed && LastCharInputed <= 'Z')
			LastCharInputed += 'a' - 'A';

		if (LastCharInputed == '\r' ||
			LastCharInputed == '\n')
		{
			if (!strcmp(Alias, ""))
				continue;
			Print("\n\n");
			if (!Load(true))
				break;
			return 1;
		}

		if (LastCharInputed == 27)
			Quit();

		if (LastCharInputed == 8 && i > 0)
		{
			Alias[i - 1] = 0;
			while (i-- > 0)
				Print("%c %c", 8, 8);
			i = strlen(Alias);
			Print("|W");
			for (int j = 0; j < nFiles; j++)
				if (!strcmp(Alias, fNames[j]))
				{
					if (fLvl[j] > 0)
						Print("|G");
					else
						Print("|R");
				}
			for (int j = 0; j < i; j++)
				Print("%c", Alias[j]);
		}

		if (LastCharInputed == '\t')
		{
			if (i == 0)
			{
				if (nFiles == 0)
					continue;
				strcpy(Alias, fNames[0]);
				i = strlen(Alias);
				for (int j = 0; j < nFiles; j++)
					if (!strcmp(Alias, fNames[j]))
					{
						if (fLvl[j] > 0)
							Print("|G");
						else
							Print("|R");
					}
				for (int k = 0; k < i; k++)
					Print("%c", Alias[k]);
				Print("|W");
			}
			for (int j = 0; j < nFiles; j++)
			{
				int k;
				if (strcmp(Alias, fNames[j]))
					for (k = 0; k < strlen(Alias); k++)
						if (Alias[k] != fNames[j][k])
							break;

				if (k == strlen(Alias))
				{
					while (i-- > 0)
					{
						Print("%c %c", 8, 8);
						Alias[i] = 0;
					}
					strcpy(Alias, fNames[j]);
					i = strlen(Alias);
					for (int j = 0; j < nFiles; j++)
						if (!strcmp(Alias, fNames[j]))
						{
							if (fLvl[j] > 0)
								Print("|G");
							else
								Print("|R");
						}
					for (int k = 0; k < i; k++)
						Print("%c", Alias[k]);
					Print("|W");
				}
			}
		}

		if (('a' <= LastCharInputed && LastCharInputed <= 'z')
			|| LastCharInputed == ' ')
		{
			if (LastCharInputed == ' ' && i == 0)
				continue;
			if (i == 0 || Alias[i - 1] == ' ' && LastCharInputed != ' ')
				LastCharInputed += 'A' - 'a';
			Alias[i++] = LastCharInputed;
			Alias[i] = 0;
			while (--i > 0)
				Print("%c %c", 8, 8);
			i = strlen(Alias);
			Print("|W");
			for (int j = 0; j < nFiles; j++)
				if (!strcmp(Alias, fNames[j]))
				{
					if (fLvl[j] > 0)
						Print("|G");
					else
						Print("|R");
				}
			for (int j = 0; j < i; j++)
				Print("%c", Alias[j]);
			Print("|W");
			if (!strcmp(Alias, "Quit") || !strcmp(Alias, "Exit"))
				Quit();
		}
	}
	return 0;
}

bool Run()
{
	ClearScreen();


	while(true)
	{
		if (TotalEnemies < NextEnemyToBeat)
			NextEnemyToBeat = TotalEnemies;
		FirstEnemyToBeat = (NextEnemyToBeat > 9) ? NextEnemyToBeat - 8 : 0;

		Save();
		CurrentHP = MaxHP;

		Print(GetSummaryStats(true, true));
		Print(" --> ");

		EnemyNumber = 0;
		LastCharInputed = 0;

		FlushInput();
		while (true)
		{
			LastCharInputed = GetChar();
/*
			PINPUT_RECORD ConInpRec;
			while (ConInpRec = GetInput())
			{
				if ((ConInpRec->EventType == KEY_EVENT) &&
					 ConInpRec->Event.KeyEvent.bKeyDown)
					if (LastCharInputed = (unsigned char)ConInpRec->Event.KeyEvent.uChar.AsciiChar)
						break;
			}
*/
			if ('A' < LastCharInputed && LastCharInputed < 'Z')
				LastCharInputed += 'a' - 'A';

			if (LastCharInputed == 10)
			{
				IsDebug = !IsDebug;

				// Refresh the screen
				//  All the extra weird code to is prevent screen flickering
/* FIXME with curses
				coord.X = 12;
				coord.Y = 2;
				SetConsoleCursorPosition(ConsoleOutput, coord);
				for (int i = 0; i < 50; i++)
					Print(" ");
				coord.X = 0;
				coord.Y = 0;
				SetConsoleCursorPosition(ConsoleOutput, coord);
*/
				Print(GetSummaryStats(true, true));
				Print(" --> ");

				Save();
			}

			// 'q'  Ctrl+C  Ctrl+Q
			else if (LastCharInputed == 27 || LastCharInputed == 'q' ||
					 LastCharInputed == 3  || LastCharInputed == 17)
				return false;

			else
			{
				LastCharInputed = CharToNumber(LastCharInputed);
				if (EnemyNumber == 0 && LastCharInputed == 0);
				else if (0 <= LastCharInputed && LastCharInputed <= 9)
					if (LastCharInputed + FirstEnemyToBeat <= NextEnemyToBeat)
					{
						// Fight the enemy
						EnemyNumber = LastCharInputed + FirstEnemyToBeat;
						Print("%c\n", NumberToChar(LastCharInputed));
						Sleep(100);
						Wait(100);
						break;
					}
					else
					{
						// Display the typed character
/* FIXME with curses
						COORD tmpc;
						tmpc.X = coord.X;
						tmpc.Y = coord.Y;
						coord.X = 0;
						coord.Y += 2;
						SetConsoleCursorPosition(ConsoleOutput, coord);
						Print("Char# = %i  ", (long)NumberToChar(LastCharInputed));
						coord.X = tmpc.X;
						coord.Y = tmpc.Y;
						SetConsoleCursorPosition(ConsoleOutput, coord);
*/
					}
			}
		}
		EnemyNumber--;

		EnemyHP = enemies[EnemyNumber]->MinHP + Random()%(enemies[EnemyNumber]->MaxHP - enemies[EnemyNumber]->MinHP);
		if (EnemyHP < 1)
			EnemyHP = 1;

		Print("\n"
			"%s has %l HP", enemies[EnemyNumber]->Name, EnemyHP);
		Wait(500);


		ClearScreen();

		while (0 < EnemyHP)
		{
			long DmgDealt = WeaponDmgBonus + Stats[STAT_STR];
			for (int i = 0; i < WeaponDice; i++)
				DmgDealt += Random()%WeaponSidesPerDie + 1;
			DmgDealt -= enemies[EnemyNumber]->ArmorClass + 1;

			// Dmg = [MinDmg, MaxDmg)
			enemiesDmgDealt = enemies[EnemyNumber]->MinDmg + Random()%(enemies[EnemyNumber]->MaxDmg - enemies[EnemyNumber]->MinDmg);

			// Armor prevents a direct amount
			enemiesDmgDealt -= ArmorClass + 1;

			// Chance to hit
			CTH = Random()%100;
			enemiesCTH = Random()%100;

			if (CTH < WeaponHitPercent)
			{
				if (0 < DmgDealt)
				{
					EnemyHP -= DmgDealt;
					if (0 < EnemyHP)
						Print("You strike the %s with your %s for |G%i point(s)|W of damage!", enemies[EnemyNumber]->Name, WeaponName, DmgDealt);
					else
						Print("You finish the %s with |G%i point(s)|W of damage!", enemies[EnemyNumber]->Name, DmgDealt);
				}
				else
					Print("Your attack is |Gdeflected harmlessly|W off the %s's %s!", enemies[EnemyNumber]->Name, enemies[EnemyNumber]->Armor);
			}
			else
				Print("Your attack |Ggoes wide|W!");
			Print("\n\n");

			if (0 < EnemyHP)
			{
				if (enemiesCTH < enemies[EnemyNumber]->WeaponHitPercent)
				{
					if (0 < enemiesDmgDealt)
					{
						CurrentHP -= enemiesDmgDealt;
						if (0 < CurrentHP)
							Print("The %s strikes you with its %s for |R%i point(s)|W of damage!", enemies[EnemyNumber]->Name, enemies[EnemyNumber]->Weapon, enemiesDmgDealt);
						else
						{
							Die();
							return true;
						}
					}
					else
						Print("The %s's %s is |Rdeflected harmlessly|W off of your %s!", enemies[EnemyNumber]->Name, enemies[EnemyNumber]->Weapon, ArmorName);
				}
				else
					Print("The %s's attack |Rgoes wide|W!", enemies[EnemyNumber]->Name);
				Print("\n\n");
			}
			else
				Print("\n\n");

			Print("HP: |B%l|W\n", CurrentHP);
			Print("%s's HP: ", enemies[EnemyNumber]->Name);
			if (EnemyHP > 0)
				Print("|B%l|W", EnemyHP);
			else
				Print("|B0|W");
			Wait(750);

			ClearScreen();
		}

		if (CurrentHP > 0)
			if (NextEnemyToBeat == EnemyNumber + 1)
				NextEnemyToBeat++;

		long long TmpExp = enemies[EnemyNumber]->Exp;
		TmpExp *= Random()%4000 + 8000;
		TmpExp /= 10000;
		Exp += TmpExp;
		Print("The %s is vanquished!\n"
			"\n"
			"You gain %i Exp\n",
			enemies[EnemyNumber]->Name, TmpExp);


		Wait(1000);
		Print("Press any key to continue . . .\n");

		PressAnyKey();
		Print("\n");

		if (ExpTNL <= Exp)
		{
			ClearScreen();
			LevelsGained = 0;
			HPGained = -GetHP();

			while (ExpTNL <= Exp)
			{
				LevelsGained++;
				Exp -= ExpTNL;
				ExpTNL += ExpNeededPerLevel * (Level + LevelsGained);
			}

			long* statsGained = new long[StatsLen];
			for (int i = 0; i < StatsLen; i++)
			{
				int oldStat = ActualStats[i];
				ActualStats[i] += cc[classID]->statGainRate[i] * LevelsGained * (1 + (Level+LevelsGained) * (Level+LevelsGained) / 100);
				statsGained[i] = (ActualStats[i]/10) - (oldStat/10);
				Stats[i] = ActualStats[i]/10 + StatBonuses[i];
			}

			ClearScreen();
			Level += LevelsGained;
			HPGained += GetHP();
			MaxHP += HPGained;
			if (LevelsGained > 1)
				Print("You gain %i levels!\n", LevelsGained);
			else
				Print("You gain a level!\n");
			Print("You are now a level %i %s!\n\n", Level, cc[classID]->name);

			for (int i = 0; i < StatsLen; i++)
				if (statsGained[i] > 0)
					Print("You gain %i %s!\n", statsGained[i], StatNames[i]);
			Print("\nYour HP raises by %i!\n\n", HPGained);
			Wait(1500);
			Print("Press any key to continue\n");

			PressAnyKey();
			Print("\n");
		}

		if (enemies[EnemyNumber]->ItemsDropped > 0)
		{
			// TODO
		}
		/*
		{
			char* TempWeaponName = (char*)malloc(strlen(enemies[EnemyNumber]->Weapon));
			strcpy(TempWeaponName, enemies[EnemyNumber]->Weapon);
			if ('a' <= TempWeaponName[0] && TempWeaponName[0] <= 'z')
				TempWeaponName[0] = TempWeaponName[0] + 'A' - 'a';
			TempWeaponDice = enemies[EnemyNumber]->WeaponDice;
			TempWeaponSidesPerDie = enemies[EnemyNumber]->WeaponSidesPerDie;
			TempWeaponDmgBonus = enemies[EnemyNumber]->WeaponDmgBonus;
			TempWeaponHitPercent = enemies[EnemyNumber]->WeaponHitPercent;

			TempWeaponMinDmg = TempWeaponDmgBonus + TempWeaponDice;
				WeaponMinDmg =		 WeaponDmgBonus +		WeaponDice;

			TempWeaponMaxDmg = TempWeaponDmgBonus + TempWeaponDice * TempWeaponSidesPerDie;
				WeaponMaxDmg =		 WeaponDmgBonus +		WeaponDice *	 WeaponSidesPerDie;

			TempWeaponTotalDmg = (TempWeaponMinDmg + TempWeaponMaxDmg) * TempWeaponHitPercent;
				WeaponTotalDmg = (		WeaponMinDmg +	 WeaponMaxDmg) *	 WeaponHitPercent;

			if (WeaponTotalDmg < TempWeaponTotalDmg)
			{
				Print("You find a(n) %s!\n"
					"Damage raised from %id%i", TempWeaponName, WeaponDice, WeaponSidesPerDie);
				if (WeaponDmgBonus > 0)
					Print(" + %i", WeaponDmgBonus);
				if (WeaponDmgBonus < 0)
					Print(" - %i", -WeaponDmgBonus);
				Print(" (%s)\n"
					"                to %id%i", WeaponName, TempWeaponDice, TempWeaponSidesPerDie);
				if (TempWeaponDmgBonus > 0)
					Print(" + %i", TempWeaponDmgBonus);
				if (TempWeaponDmgBonus < 0)
					Print(" - %i", -TempWeaponDmgBonus);

				if (Level/GAIN_STR_INTERVAL != 0)
					Print(" (%s)\n\nYour strength bonus also gives you an additional %i damage\n",
						TempWeaponName, Stats[STAT_STR]);

				if (TempWeaponHitPercent - WeaponHitPercent > 0)
					Print("\nAlso, you feel more accurate with this weapon\n");
				else if (TempWeaponHitPercent - WeaponHitPercent < 0)
					Print("\nUnfortunatly, you feel less accurate with this weapon\n");
				WeaponHitPercent = TempWeaponHitPercent;

				strcpy(WeaponName,	enemies[EnemyNumber]->Weapon);
				WeaponDice			= enemies[EnemyNumber]->WeaponDice;
				WeaponSidesPerDie	= enemies[EnemyNumber]->WeaponSidesPerDie;
				WeaponDmgBonus		= enemies[EnemyNumber]->WeaponDmgBonus;
				WeaponHitPercent	= enemies[EnemyNumber]->WeaponHitPercent;
				WeaponTotalDmg		= TempWeaponTotalDmg;

				Print("\n");

				Wait(1000);
				Print("Press any key to continue . . .\n");
				PressAnyKey();
				Print("\n\n");
			}
		}


		if (enemies[EnemyNumber]->ArmorDropped == true && Random()%4 == 0)
		{
			char* TempArmorName = (char*)malloc(strlen(enemies[EnemyNumber]->Armor));
			strcpy(TempArmorName, enemies[EnemyNumber]->Armor);
			if ('a' <= TempArmorName[0] && TempArmorName[0] <= 'z')
				TempArmorName[0] = TempArmorName[0] + 'A' - 'a';
			TempArmorClass = enemies[EnemyNumber]->ArmorClass;
			if (ArmorClass < TempArmorClass)
			{
				ArmorClassDiff = TempArmorClass - ArmorClass;

				ArmorClass = TempArmorClass;
				strcpy(ArmorName, TempArmorName);

				Print("The %s's %s is still in working condition!\n"
					"AC raised by %i!\n"
					"New AC is %i!\n",
					enemies[EnemyNumber]->Name, TempArmorName, ArmorClassDiff,
					TempArmorClass);

				Wait(1000);
				Print("\nPress any key to continue . . .");

				PressAnyKey();
				Print("\n\n");
			}
		}
		*/
		Print("\n");
		ClearScreen();
	}
}



int main(int argc, char* argv[])
//int WINAPI WinMain(HINSTANCE hThisInstance,
//				   HINSTANCE hPrevInstance,
//				   LPSTR lpszArgument,
//				   int nFunsterStil)
{
//	hInstance = hThisInstance;
//	g_hWnd = null;
/*
	if (!InitEngine())
		return 1;
*/
//	CreateTheWindow(nFunsterStil);

	HandleArgs(argc, argv);

	CanQuit = true;

	// Load everything
	Print("|WLoading data...");
	LoadData();

	if (Alias[0] != null)
		if (!Load(true))
			while(true)
				Run();

	while(true)
	{
		while (MainMenu())
			ClearScreen();
		while (Run());
	}
}

