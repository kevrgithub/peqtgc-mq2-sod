// MQ2DataStructures.cpp : Defines the entry point for the DLL application.
//

// LOG
// 9/14/2010: Reno - Initial release.


// GIVE ME COMMENTS OR GIVE ME DEATH

#include "../MQ2Plugin.h"
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <sstream>
#include <algorithm>

PreSetup("MQ2DataStructures");

/********************************************************************************
*********************************************************************************
BEGIN TYPE VERIFICATION
*********************************************************************************
********************************************************************************/
bool StringIsInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return true;
}

int StringToInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return integer;
}

bool StringIsFloat(std::string value) {
	std::istringstream iss(value);
	float floating = 0;

	if(!(iss >> floating))
		return false;
	else
		return true;
}

float StringToFloat(std::string value) {
	std::istringstream iss(value);
	float floating = 0;

	if(!(iss >> floating))
		return false;
	else
		return floating;
}

bool StringIsBool(std::string value) {
	if(value == "true" || value == "TRUE" || value == "false" || value == "FALSE" || value == "null" || value == "NULL")
		return true;

	return false;
}

bool StringToBool(std::string value) {
	if(value == "true" || value == "TRUE") 
		return true;

	return false;
}

std::string NumericToString(int a) {
	std::string s;
	std::stringstream out;
	out << a;
	s = out.str();
	return s;
}

std::string NumericToString(float a) {
	std::string s;
	std::stringstream out;
	out << a;
	s = out.str();
	return s;
}
/*****************************
******************************
END TYPE VERIFICATION
******************************
*****************************/

//What to store these in. A string must somehow get us to a container.....
//map of <string, * container> for each of the differing types?

//need one container to hold the variables and there possible linkages??
/*********************************************************
**********************************************************
BEGIN HASHMAP FUNCTIONALITY
**********************************************************
*********************************************************/
class MQ2HashMapType *pHashMapType = 0;

map<std::string, map<std::string, std::string> > hashVars;
typedef pair<std::string, map<std::string, std::string> > hash_pair;
typedef map<std::string, map<std::string, std::string> >::iterator hash_iter;

//HashMap[mapname]
//HashMap[mapname].Insert[key,value]  <=== better way?
//HashMap[mapname].Find[key]
//HashMap[mapname].Erase
//HashMap[mapname].Size
//HashMap[mapname].IsEmpty
// A way to iterate through the hash, need to find an easy way. In real life, we use Iterator's, this is not an option here
// A way to delete specific key/value pairs
// TODO: Spaces in Insert need to be handled
        //[key , value]
		//[key, value]
		//[key ,value]
		//[ key, value ]
        //[ key , value ]
class MQ2HashMapType : public MQ2Type {
private:
	map<std::string, std::string>* hash;
	std::string varName;

public:
	enum HashMapMembers {
		Insert=1,
		Find=2,
		Erase=3,
		Size=4,
		IsEmpty=5,
		Remove=6
	};

	MQ2HashMapType():MQ2Type("HashMap") {
		TypeMember(Insert);
		TypeMember(Find);
		TypeMember(Erase);
		TypeMember(Size);
		TypeMember(IsEmpty);
		TypeMember(Remove);
	}

	~MQ2HashMapType() {}

	void SetName(std::string name) {
		varName = name;
	}

	void SetHash(map<std::string, std::string>* theHash) {
		hash = theHash;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2HashMapType::FindMember(Member);
		if (!pMember)
			return false;

		std::string key, value;
		pair<std::string, std::string> myPair;
		int comma = 0;
		//char szTemp[1024] = {0};
		switch((HashMapMembers)pMember->ID) {
			case Insert:
				if(strcmp(Index, "") == 0 || (!strchr(Index, 44)))
					return false;

				for(int i = 0; i < strlen(Index); ++i) {
					if(Index[i] == ',') {
						comma = i;
						break;
					} else
						key += Index[i];
				}

				for(int i = comma + 1; i < strlen(Index); ++i) 
					value += Index[i];

				//remove any extra spaces at beginnings or ends
				while(key[0] == ' ' && key.size() > 1)
					key = key.substr(1, key.size());

				while(key[key.size()-1] == ' ' && key.size() > 1)
					key = key.substr(0, key.size()-1);
				
				while(value[0] == ' ' && value.size() > 1)
					value = value.substr(1, value.size());

				while(value[value.size()-1] == ' ' && value.size() > 1)
					value = value.substr(0, value.size()-1);
				//sprintf(szTemp, "KEY:[%s]", key.c_str());
				//WriteChatColor(szTemp);
				//sprintf(szTemp, "VAL:[%s]", value.c_str());
			//	WriteChatColor(szTemp);
				myPair.first = key;
				myPair.second = value;

				hash->insert(myPair);
				Dest.DWord = true;
				Dest.Type = pBoolType;
				return true;
			case Find:
				if(strcmp(Index, "") != 0) {
					map<std::string, std::string>::iterator it = hash->find(Index);
					if(it != hash->end()) {
						Dest.Type = pStringType;
						std::string temp = it->second;
						//sprintf(szTemp, "FOUND:[%s]", temp.c_str());
						//WriteChatColor(szTemp);
						strcpy(DataTypeTemp, temp.c_str());
						Dest.Ptr = DataTypeTemp;
						return true;
					} else 
						return false;
				}
				return false;
			case Erase:
				hash->clear();
				return true;
			case Size:
				Dest.Int = hash->size();
				Dest.Type = pIntType;
				return true;
			case IsEmpty:
				Dest.DWord = hash->empty();
				Dest.Type = pBoolType;
				return true;
			case Remove:
				if(strcmp(Index, "") != 0) {
					map<std::string, std::string>::iterator it = hash->find(Index);
					if(it != hash->end()) {
						hash->erase(it);
						Dest.DWord = true;
						Dest.Type = pBoolType;
					} else {
						Dest.DWord = false;
						Dest.Type = pBoolType;
					}
				}
				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination, varName.c_str());

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

map<std::string, std::string>* FindHash(PCHAR szName) {
	hash_iter it;
	std::string name = szName;
	for(it = hashVars.begin(); it != hashVars.end(); ++it) {
		if(name == it->first) {
			//WriteChatColor("VECTOR FOUND");
			return (&it->second);
		}
	}
}

bool VarExistsInHash(PCHAR szName) {
	hash_iter it;
	std::string name = szName;
	for(it = hashVars.begin(); it != hashVars.end(); ++it) {
		if(name == it->first)
			return true;
	}

	return false;
}

hash_pair AddVarToHash(PCHAR szName) {
	hash_pair hashPair;
	map<std::string, std::string> hash;
	hashPair.first = szName;
	hashPair.second = hash;
	hashVars.insert(hashPair);

	return hashPair;
}

BOOL dataHashMap(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pHashMapType;

	if(strcmp(szName, "") == 0)
		return false;

	if(VarExistsInHash(szName)) {
		//WriteChatColor("USING EXISTING VARIABLE");
	} else {
		//WriteChatColor("ADDING VARIABLE");
		hash_pair myPair = AddVarToHash(szName);
	}

	map<std::string, std::string>* hash = FindHash(szName);
	pHashMapType->SetName(szName);
	pHashMapType->SetHash(hash);	

	return true;
}

/*************************************
**************************************
END HASHMAP
**************************************
*************************************/

/*********************************************************
**********************************************************
BEGIN VECTOR FUNCTIONALITY
**********************************************************
*********************************************************/
class MQ2VectorType *pVectorType = 0;

map<std::string, vector<std::string> > vectorVars;
typedef pair<std::string, vector<std::string> > vec_pair;
typedef map<std::string, vector<std::string> >::iterator vec_iter;

bool IntAsc(int i, int j) { return (i<j); }
bool IntDesc(int i, int j) { return (i>j); }
bool StringDesc(std::string a, std::string b) { return (a>b); }

//Vector[vectorname]
//Vector[vectorname].At[index]
//Vector[vectorname].Size
//Vector[vectorname].Insert[value]
//Vector[vectorname].Erase
//Vector[vectorname].Remove[index]
//Vector[vectorname].IsEmpty
//Vector[vectorname].Sort
class MQ2VectorType : public MQ2Type {
private:
	vector<std::string>* vec;
	std::string varName;

public:
	enum VectorMembers {
		At=1,
		Size=2,
		Insert=3,
		Erase=4,
		Remove=5,
		IsEmpty=6,
		Sort=7
	};

	MQ2VectorType():MQ2Type("Vector") {
		TypeMember(At);
		TypeMember(Size);
		TypeMember(Insert);
		TypeMember(Erase);
		TypeMember(Remove);
		TypeMember(IsEmpty);
		TypeMember(Sort);

		varName = "EMPTY";
	}

	~MQ2VectorType() {}

	void SetName(std::string name) {
		varName = name;
	}

	void SetVector(vector<std::string>* theVector) {
		vec = theVector;
	}

	bool IsOnlyNumbers() {
		bool bAllNums = true;
		int size = vec->size();
		for(int i = 0; i < size; ++i) {
			std::string value = vec->at(i);
			if(!StringIsInt(value) || !StringIsFloat(value)) {
				bAllNums = false;
				break;
			}
		}

		return bAllNums;
	}

	void SortNumerics(bool desc = false) {
		vector<int> ints;
		int size = vec->size();
		for(int i = 0; i < size; ++i) {
			std::string value = vec->at(i);
			if(StringIsInt(value))
				ints.push_back(StringToInt(value));
			else if(StringIsFloat(value))
				ints.push_back(StringToFloat(value));
		}

		vec->clear();

		sort(ints.begin(), ints.end(), (desc) ? IntDesc : IntAsc);

		for(int i = 0; i < size; ++i)
			vec->push_back(NumericToString(ints.at(i)));
	}

	void PrintVector() {
		WriteChatColor("VECTOR:");
		for(int i = 0; i < vec->size(); ++i) {
			std::string val = "[" + vec->at(i) + "]";
			char szTemp[1024] = {0};
			sprintf(szTemp, "%s", val.c_str());
			WriteChatColor(szTemp);
		}
		WriteChatColor("END");
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2VectorType::FindMember(Member);
		if (!pMember)
			return false;
		switch((VectorMembers)pMember->ID) {
			case At:
				//check index for number, if it isn't an integer this is SHIT
				if(StringIsInt(Index)) {
					int index = StringToInt(Index);
					if(vec->size() > index) {
						Dest.Ptr = strcpy(DataTypeTemp, vec->at(index).c_str());
						Dest.Type = pStringType;
						return true;
					}
				} 
				//PrintVector();
				return false;
			case Size:
				Dest.Int = (int)vec->size();
				Dest.Type = pIntType;
				//PrintVector();
				return true;
			case Insert:
				vec->push_back(Index);
				//WriteChatColor("ADDED TO VECTOR");
				//WriteChatColor(Index);
				//PrintVector();
				Dest.DWord = true;
				Dest.Type = pBoolType;

				return true;
			case Erase:
				vec->clear();
				//PrintVector();
				return true;
			case Remove:
				//has to be an integer
				if(StringIsInt(Index)) {
					int index = StringToInt(Index);
					if(vec->size() > index) {
						vec->erase(vec->begin() + index);
						return true;
					}
				}
				//PrintVector();
				return false;
			case IsEmpty:
				Dest.DWord = vec->empty();
				Dest.Type = pBoolType;
				return true;
			case Sort:
				bool bDesc = false;
				if(strcmp(Index, "desc") == 0 || strcmp(Index, "DESC") == 0 || strcmp(Index, "descending") == 0 || strcmp(Index, "DESCENDING") == 0)
						bDesc = true;

				if(IsOnlyNumbers()) {					
					SortNumerics(bDesc);
				} else {
					if(bDesc)
						sort(vec->begin(), vec->end(), StringDesc);
					else
						sort(vec->begin(), vec->end());
				}

				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination, varName.c_str());

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

vector<std::string>* FindVector(PCHAR szName) {
	vec_iter it;
	std::string name = szName;
	for(it = vectorVars.begin(); it != vectorVars.end(); ++it) {
		if(name == it->first) {
			//WriteChatColor("VECTOR FOUND");
			return (&it->second);
		}
	}
}

bool VarExists(PCHAR szName) {
	vec_iter it;
	std::string name = szName;
	for(it = vectorVars.begin(); it != vectorVars.end(); ++it) {
		if(name == it->first)
			return true;
	}

	return false;
}

vec_pair AddVar(PCHAR szName) {
	vec_pair vecPair;
	vector<std::string> vec;
	vecPair.first = szName;
	vecPair.second = vec;
	vectorVars.insert(vecPair);

	return vecPair;
}

BOOL dataVector(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pVectorType;

	if(strcmp(szName, "") == 0)
		return false;

	if(VarExists(szName)) {
		//WriteChatColor("USING EXISTING VARIABLE");
	} else {
		//WriteChatColor("ADDING VARIABLE");
		vec_pair myPair = AddVar(szName);
	}

	vector<std::string>* vec = FindVector(szName);
	pVectorType->SetName(szName);
	pVectorType->SetVector(vec);	

	return true;
}
/*************************************
**************************************
END VECTOR
**************************************
*************************************/

/*********************************************************
**********************************************************
BEGIN QUEUE FUNCTIONALITY
**********************************************************
*********************************************************/
class MQ2QueueType *pQueueType = 0;

map<std::string, queue<std::string> > queueVars;
typedef pair<std::string, queue<std::string> > que_pair;
typedef map<std::string, queue<std::string> >::iterator que_iter;

//Queue[qname]
//Queue[qname].Push[value]
//Queue[qname].Pop[value]
//Queue[qname].Peek[value]
//Queue[qname].Erase
//Queue[qname].Size
//Queue[qname].Back
//Queue[qname].IsEmpty
class MQ2QueueType : public MQ2Type {
private:
	queue<std::string>* que;
	std::string varName;

public:
	enum QueueMembers {
		Push=1,
		Pop=2,
		Peek=3,
		Erase=4,
		Size=5,
		Back=6,
		IsEmpty=7
	};

	MQ2QueueType():MQ2Type("Queue") {
		TypeMember(Push);
		TypeMember(Pop);
		TypeMember(Peek);
		TypeMember(Erase);
		TypeMember(Size);
		TypeMember(Back);
		TypeMember(IsEmpty);

		varName = "EMPTY";
	}

	~MQ2QueueType() {}

	void SetName(std::string name) {
		varName = name;
	}

	void SetQueue(queue<std::string>* theQueue) {
		que = theQueue;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2QueueType::FindMember(Member);
		if (!pMember)
			return false;
		switch((QueueMembers)pMember->ID) {
			case Push:
				if(strcmp(Index, "") != 0) {
					que->push(Index);
					Dest.DWord = true;
					Dest.Type = pBoolType;
					return true;
				}

				return false;
			case Pop:
				if(!que->empty()) {
					Dest.Type = pStringType;
					strcpy(DataTypeTemp, que->front().c_str());
					Dest.Ptr = DataTypeTemp;
					que->pop();
					return true;
				}

				return false;
			case Peek:
				if(!que->empty()) {
					Dest.Type = pStringType;
					strcpy(DataTypeTemp, que->front().c_str()); 
					Dest.Ptr = DataTypeTemp;
					return true;
				}
				return false;
			case Erase:
				while(!que->empty())
					que->pop();

				return true;
			case Size:
				if(que->size() > 0)
					Dest.Int = que->size();
				else 
					Dest.Int = 0;

				Dest.Type = pIntType;
				return true;
			case Back:
				if(!que->empty()) {
					Dest.Type = pStringType;
					strcpy(DataTypeTemp, que->back().c_str()); 
					Dest.Ptr = DataTypeTemp;
					return true;
				}
				return false;
			case IsEmpty:
				Dest.DWord = que->empty();
				Dest.Type = pBoolType;
				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination, varName.c_str());

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

queue<std::string>* FindQueue(PCHAR szName) {
	que_iter it;
	std::string name = szName;
	for(it = queueVars.begin(); it != queueVars.end(); ++it) {
		if(name == it->first) {
			//WriteChatColor("VECTOR FOUND");
			return (&it->second);
		}
	}
}

bool VarExistsInQueue(PCHAR szName) {
	que_iter it;
	std::string name = szName;
	for(it = queueVars.begin(); it != queueVars.end(); ++it) {
		if(name == it->first)
			return true;
	}

	return false;
}

que_pair AddVarToQueue(PCHAR szName) {
	que_pair quePair;
	queue<std::string> que;
	quePair.first = szName;
	quePair.second = que;
	queueVars.insert(quePair);

	return quePair;
}

BOOL dataQueue(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pQueueType;

	if(strcmp(szName, "") == 0)
		return false;

	if(VarExistsInQueue(szName)) {
		//WriteChatColor("USING EXISTING VARIABLE");
	} else {
		//WriteChatColor("ADDING VARIABLE");
		que_pair myPair = AddVarToQueue(szName);
	}

	queue<std::string>* que = FindQueue(szName);
	pQueueType->SetName(szName);
	pQueueType->SetQueue(que);	

	return true;
}

/*************************************
**************************************
END QUEUE FUNCTIONALITY
**************************************
*************************************/

/*********************************************************
**********************************************************
BEGIN STACK FUNCTIONALITY
**********************************************************
*********************************************************/
class MQ2StackType *pStackType = 0;

map<std::string, stack<std::string> > stackVars;
typedef pair<std::string, stack<std::string> > stk_pair;
typedef map<std::string, stack<std::string> >::iterator stk_iter;

//Stack[sname]
//Stack[sname].Push[value]
//Stack[sname].Pop
//Stack[sname].Peek
//Stack[sname].Size
//Stack[sname].Last
//Stack[sname].IsEmpty
class MQ2StackType : public MQ2Type {
private:
	stack<std::string>* stk;
	std::string varName;

public:
	enum StackMembers {
		Push=1,
		Pop=2,
		Peek=3,
		Erase=4,
		Size=5,
		IsEmpty=6
	};

	MQ2StackType():MQ2Type("Stack") {
		TypeMember(Push);
		TypeMember(Pop);
		TypeMember(Peek);
		TypeMember(Erase);
		TypeMember(Size);
		TypeMember(IsEmpty);
	}

	~MQ2StackType() {}

	void SetName(std::string name) {
		varName = name;
	}

	void SetStack(stack<std::string>* theStack) {
		stk = theStack;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2StackType::FindMember(Member);
		if (!pMember)
			return false;
		switch((StackMembers)pMember->ID) {
			case Push:
				if(strcmp(Index, "") != 0) {
					stk->push(Index);
					Dest.DWord = true;
					Dest.Type = pBoolType;
					return true;
				}

				return false;
			case Pop:
				if(!stk->empty()) {
					Dest.Type = pStringType;
					strcpy(DataTypeTemp, stk->top().c_str());
					Dest.Ptr = DataTypeTemp;
					stk->pop();
					return true;
				}

				return false;
			case Peek:
				if(!stk->empty()) {
					Dest.Type = pStringType;
					strcpy(DataTypeTemp, stk->top().c_str()); 
					Dest.Ptr = DataTypeTemp;
					return true;
				}
				return false;
			case Erase:
				while(!stk->empty())
					stk->pop();

				return true;
			case Size:
				if(stk->size() > 0)
					Dest.Int = stk->size();
				else 
					Dest.Int = 0;

				Dest.Type = pIntType;
				return true;
			case IsEmpty:
				Dest.DWord = stk->empty();
				Dest.Type = pBoolType;
				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination, varName.c_str());

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

stack<std::string>* FindStack(PCHAR szName) {
	stk_iter it;
	std::string name = szName;
	for(it = stackVars.begin(); it != stackVars.end(); ++it) {
		if(name == it->first) {
			//WriteChatColor("VECTOR FOUND");
			return (&it->second);
		}
	}
}

bool VarExistsInStack(PCHAR szName) {
	stk_iter it;
	std::string name = szName;
	for(it = stackVars.begin(); it != stackVars.end(); ++it) {
		if(name == it->first)
			return true;
	}

	return false;
}

stk_pair AddVarToStack(PCHAR szName) {
	stk_pair stkPair;
	stack<std::string> stk;
	stkPair.first = szName;
	stkPair.second = stk;
	stackVars.insert(stkPair);

	return stkPair;
}

BOOL dataStack(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pStackType;

	if(strcmp(szName, "") == 0)
		return false;

	if(VarExists(szName)) {
		//WriteChatColor("USING EXISTING VARIABLE");
	} else {
		//WriteChatColor("ADDING VARIABLE");
		stk_pair myPair = AddVarToStack(szName);
	}

	stack<std::string>* stk = FindStack(szName);
	pStackType->SetName(szName);
	pStackType->SetStack(stk);	

	return true;
}

/*************************************
**************************************
END STACK
**************************************
*************************************/

/************************************
*************************************
BEGIN SLASH COMMAND
*************************************
************************************/
VOID DataCommand(PSPAWNINFO pChar, PCHAR szLine) {
	//this is just so we have a valid slash command and don't waste cycles using /echo or some shit
}
/************************************
*************************************
END SLASH COMMAND
*************************************
************************************/
// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2DataStructures");

	AddMQ2Data("Vector", dataVector);
	pVectorType = new MQ2VectorType;

	AddMQ2Data("Queue", dataQueue);
	pQueueType = new MQ2QueueType;

	AddMQ2Data("Stack", dataStack);
	pStackType = new MQ2StackType;

	AddMQ2Data("HashMap", dataHashMap);
	pHashMapType = new MQ2HashMapType;

	AddCommand("/data", DataCommand);
	// Add commands, MQ2Data items, hooks, etc.
	// AddCommand("/mycommand",MyCommand);
	// AddXMLFile("MQUI_MyXMLFile.xml");
	// bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2DataStructures");

	RemoveMQ2Data("Vector");
	delete pVectorType;

	RemoveMQ2Data("Queue");
	delete pQueueType;

	RemoveMQ2Data("Stack");
	delete pStackType;

	RemoveMQ2Data("HashMap");
	delete pHashMapType;

	RemoveCommand("/data");
	// Remove commands, MQ2Data items, hooks, etc.
	// RemoveMQ2Benchmark(bmMyBenchmark);
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
}
