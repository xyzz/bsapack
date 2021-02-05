#if !defined(AFX_BSAPACK_H__21B07D82_CB81_11D2_A5B3_00403353FDBD__INCLUDED_)
#define AFX_BSAPACK_H__21B07D82_CB81_11D2_A5B3_00403353FDBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
// #include <windows.h>

#define MAX_FILE_NUM 20000

typedef struct _Header_ {
	unsigned version;
	unsigned hashOffset;
	unsigned number;
} BSAHeader;

typedef struct _FileInfo_ {
	unsigned size;
	unsigned offset;
} FileInfo;

typedef struct _HashRecord_ {
	unsigned value1;
	unsigned value2;
} HashRecord;

class BSARecord {
	char *name;
	unsigned len;
	FileInfo fileInfo;
	HashRecord hash;

	void calculateHash();

public:
	~BSARecord();
	void SetName(char *pName);
	void SetFileInfo(unsigned pSize, unsigned pOffset);

	char *GetName() {
		return name;
	}

	unsigned GetLength() {
		return len;
	}

	unsigned GetOffset() {
		return fileInfo.offset;
	}

	unsigned GetSize() {
		return fileInfo.size;
	}

	FileInfo GetFileInfo() {
		return fileInfo;
	}

	HashRecord GetHash() {
		return hash;
	}

	int ValidateHash(HashRecord *pHash) {
		return ((hash.value1 == pHash->value1) && (hash.value2 == pHash->value2));
	}

	static int CompareByHash(const void *pElem1, const void *pElem2);
	static int CompareByName(const void *pElem1, const void *pElem2);
};

class BSAFile {
	char *name;
	BSAHeader header;
	BSARecord *files;

	void readFile();
    void createFile(char **pFileList, unsigned pNum);
	void prepareTables(char *pBuffer);

public:
	BSAFile (char *pName);
	BSAFile (char *pName, char **pFileList, unsigned pNum);
	void PrintContent();
	void UnpackFiles();
	void PackFiles();
	~BSAFile();
};


#endif // !defined(AFX_BSAPACK_H__21B07D82_CB81_11D2_A5B3_00403353FDBD__INCLUDED_)
