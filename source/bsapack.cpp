#include "bsapack.h"

// --------------------------------------------------------------------------

void BSARecord::SetName(char *pName) {
	unsigned i;

	len = strlen(pName);
	name = new char[len+1];
	strcpy(name, pName);
	
	for (i = 0; i < len; i++)
		name[i] = tolower(name[i]);

	calculateHash();
}

// --------------------------------------------------------------------------

void BSARecord::SetFileInfo(unsigned pSize, unsigned pOffset) {
	fileInfo.size = pSize;
	fileInfo.offset = pOffset;
}

// --------------------------------------------------------------------------

BSARecord::~BSARecord() {
	if (name)
		delete[] name;
}

// --------------------------------------------------------------------------

void BSARecord::calculateHash() {
	unsigned l = (len>>1);
	unsigned sum, off, temp, i, n;

	for(sum = off = i = 0; i < l; i++) {
		sum ^= (((unsigned)(name[i]))<<(off&0x1F));
		off += 8;
	}
	hash.value1 = sum;

	for(sum = off = 0; i < len; i++) {
		temp = (((unsigned)(name[i]))<<(off&0x1F));
		sum ^= temp;
		n = temp & 0x1F;
		sum = (sum << (32-n)) | (sum >> n);  // binary "rotate right"
		off += 8;
	}
	hash.value2 = sum;
}

// --------------------------------------------------------------------------

int BSARecord::CompareByHash(const void *pElem1, const void *pElem2) {
	BSARecord *elem1 = (BSARecord *)pElem1;
	BSARecord *elem2 = (BSARecord *)pElem2;
	int result;

	if ( (elem1->hash.value1) < (elem2->hash.value1) ) {
		return -1;
	}
	else if ( (elem1->hash.value1) > (elem2->hash.value1) ) {
		return 1;
	}
	else if ( (elem1->hash.value2) < (elem2->hash.value2) ) {
		return -1;
	}
	else if ( (elem1->hash.value2) > (elem2->hash.value2) ) {
		return 1;
	}
	
	return stricmp(elem1->name, elem2->name);
}

// --------------------------------------------------------------------------

int BSARecord::CompareByName(const void *pElem1, const void *pElem2) {
	BSARecord *elem1 = (BSARecord *)pElem1;
	BSARecord *elem2 = (BSARecord *)pElem2;

	return stricmp(elem1->name, elem2->name);
}

// --------------------------------------------------------------------------

BSAFile::BSAFile (char *pName) {
	name = new char[strlen(pName)+1];
	strcpy(name, pName);

	readFile();
}

// --------------------------------------------------------------------------

BSAFile::BSAFile (char *pName, char **pFileList, unsigned pNum) {
	name = new char[strlen(pName)+1];
	strcpy(name, pName);

	createFile(pFileList, pNum);
}

// --------------------------------------------------------------------------

BSAFile::~BSAFile () {
	delete[] name;
	delete[] files;
	
}

// --------------------------------------------------------------------------

void BSAFile::createFile(char **pFileList, unsigned pNum) {
	unsigned i;
	struct _stat fileStat;
	unsigned offset;

	header.version = 0x00000100;
	header.number = pNum;

	files = new BSARecord[pNum];

	for(i=0; i < pNum; i++) {
		files[i].SetName(pFileList[i]);
	}

	qsort(files, pNum, sizeof(BSARecord), BSARecord::CompareByName);

	offset = 0;
	for(i=0; i < pNum; i++) {
		_stat(files[i].GetName(), &fileStat);
		files[i].SetFileInfo(fileStat.st_size, offset);
		offset += fileStat.st_size;
	}

	qsort(files, pNum, sizeof(BSARecord), BSARecord::CompareByHash);

	offset = (sizeof(FileInfo) + sizeof(unsigned))*pNum;
	for(i=0; i < pNum; i++)
		offset += (files[i].GetLength() + 1);
	header.hashOffset = offset;
}

// --------------------------------------------------------------------------

void BSAFile::readFile() {
	FILE *f = fopen(name, "rb");
	char *aBuffer;
	FileInfo *fileInfoList;
	unsigned *dirOffsetList;
	char *directory;
	HashRecord *hashList;
	unsigned i;

	if (f == NULL) {
		printf("FATAL: Cannot open file %s\n", name);
		exit(1);
	}

	fread(&header, sizeof(BSAHeader), 1, f);

	if (header.version != 0x00000100) {
		printf("FATAL: Unsupported version %0X of the BSA file\n", header.version);
		exit(1);
	}

	aBuffer = new char[header.hashOffset + header.number*sizeof(HashRecord)];
	files = new BSARecord[header.number];

	fread(aBuffer, sizeof(char), header.hashOffset + header.number*sizeof(HashRecord), f);
	fclose(f);

	fileInfoList = (FileInfo *)aBuffer;
	dirOffsetList = (unsigned *)(aBuffer + header.number*sizeof(FileInfo));
	directory = aBuffer + header.number*sizeof(FileInfo) + header.number*sizeof(unsigned);
	hashList = (HashRecord *)(aBuffer + header.hashOffset);

	for (i = 0; i < header.number; i++ ) {
		files[i].SetName(directory + dirOffsetList[i]);
		files[i].SetFileInfo(fileInfoList[i].size, fileInfoList[i].offset);
		if (!files[i].ValidateHash(hashList+i)) {
			printf("ERROR: Hash failed validation for %s\n", directory + dirOffsetList[i]);
		}
	}

	delete[] aBuffer;
}

// --------------------------------------------------------------------------

void BSAFile::PrintContent() {
	unsigned i;
	HashRecord hash;

	printf("Archive content:\n\n");
	printf("%10s %8s  %8s %8s  %s\n", "Offset","Size","Hash1", "Hash2", "Name");
	printf("%10s %8s  %8s %8s  %s\n", "----------","--------","--------","--------","--------------------------------------");
	for(i = 0; i < header.number; i++) {
		hash = files[i].GetHash();
		printf("%10u %8u  %08X %08X  %s\n", files[i].GetOffset(), files[i].GetSize(), hash.value1, hash.value2, files[i].GetName());
	}
/*
	qsort(files, header.number, sizeof(BSARecord), BSARecord::CompareByName);

	printf("\n\n\n\nOrdered file content:\n\n");
	printf("%10s  %10s  %s\n", "Offset","Size","Name");
	printf("%10s  %10s  %s\n", "----------","----------","----------------------------------------");
	for(i = 0; i < header.number; i++) {
		printf("%10u  %10u  %s\n", files[i].GetOffset(), files[i].GetSize(), files[i].GetName());
	}
*/

}

// --------------------------------------------------------------------------

FILE *openFile(char *pName) {
	char name[1000], *ptr;
	FILE *f;

	strcpy(name, pName);
	if(!_access(name, 0)) {
		printf("ERROR: File %s already exists\n", name);
		exit(1);
	}
	ptr = strrchr(name, '\\');
	if(!ptr) {
		printf("ERROR: Cannot parse path in %s", name);
		exit(1);
	}

	ptr[0] = '\0';
	if (_access(name, 0)) {
		printf("Creating directories for %s\n", pName);
		strcpy(name, pName);
		ptr = name;
		while((ptr = strchr(ptr+1,'\\')) != NULL) {
			ptr[0]='\0';
			if(_access(name, 0))
				_mkdir(name);
			ptr[0]='\\';
		}
	}
	if((f = fopen(pName, "wb")) == NULL) {
		printf("ERROR: Can not create file %s\n", pName);
		exit(1);
	}
	return f;
}

// --------------------------------------------------------------------------


void BSAFile::UnpackFiles() {
	unsigned i;
	unsigned offset;
	unsigned size;
	unsigned max;
	FILE *src, *tgt;
	char *aBuffer;


	printf("Unpacking %u files...\n", header.number);

	max = 0;
	for(i = 0; i < header.number; i++) {
		if (max < files[i].GetSize())
			max = files[i].GetSize();
	}
	printf("Allocating %u bytes for the buffer\n", max);
	aBuffer = new char[max];

	if((src = fopen(name, "rb")) == NULL) {
		printf("FATAL: Cannot open file %s\n", name);
		exit(1);
	}
	
	for(i = 0; i < header.number; i++) {
		offset = files[i].GetOffset() + header.hashOffset + sizeof(BSAHeader) + sizeof(HashRecord)*header.number;
		size = files[i].GetSize();
		printf("Extracting %s  (%u bytes)\n", files[i].GetName(), size);
		tgt = openFile(files[i].GetName());
		if(fseek(src, offset, SEEK_SET)) {
			printf("ERROR: Invalid file offset %0X for the file %s\n", offset, files[i].GetName());
			exit(1);
		}
		if(fread(aBuffer, sizeof(char), size, src) != size) {
			printf("ERROR: error during reading %s\n", files[i].GetName());
			exit(1);
		}
		if(fwrite(aBuffer, sizeof(char), size, tgt) != size) {
			printf("ERROR: error during writing %s\n", files[i].GetName());
			exit(1);
		}
		fclose(tgt);
	}
	fclose(src);
	delete[] aBuffer;
}

// --------------------------------------------------------------------------

unsigned scanFiles(char **pFileList, char *pPath, unsigned pNum) {
	intptr_t fhandle;
	struct _finddata_t fdata;
	char mask[500];
	char name[500];

	if(pPath[0]) {
		strcpy(mask, pPath);
		strcat(mask, "\\*.*");
	}
	else {
		strcpy(mask, "*.*");
	}

	if((fhandle = _findfirst(mask, &fdata)) == -1L)
		return pNum;
	
	do {
		if (fdata.name[0] == '.')
			continue;
		
		if (fdata.attrib & _A_SUBDIR) {
			if(pPath[0]) {
				strcpy(name, pPath);
				strcat(name, "\\");
				strcat(name, fdata.name);
				pNum = scanFiles(pFileList, name, pNum);
			}
			else {
				pNum = scanFiles(pFileList, fdata.name, pNum);
			}
		}
		else if(pPath[0]) {
			strcpy(name, pPath);
			strcat(name, "\\");
			strcat(name, fdata.name);
			if(pNum >= MAX_FILE_NUM) {
				printf("ERROR: too many files - current version supports only %u files per archive\n", MAX_FILE_NUM);
				exit(1);
			}
			pFileList[pNum] = new char[strlen(name)+1];
			strcpy(pFileList[pNum], name);
			pNum++;
		}
	} while( _findnext(fhandle, &fdata) == 0);

	return pNum;
}

// --------------------------------------------------------------------------

void BSAFile::prepareTables(char *pBuffer) {
	unsigned i;
	unsigned dirOffset;
	FileInfo *fileInfoList;
	unsigned *dirOffsetList;
	char *directory;
	HashRecord *hashList;

	fileInfoList = (FileInfo *)pBuffer;
	dirOffsetList = (unsigned *)(pBuffer + header.number*sizeof(FileInfo));
	directory = pBuffer + header.number*sizeof(FileInfo) + header.number*sizeof(unsigned);
	hashList = (HashRecord *)(pBuffer + header.hashOffset);

	dirOffset = 0;
	for (i = 0; i < header.number; i++ ) {
		strcpy(directory+dirOffset, files[i].GetName());
		dirOffsetList[i] = dirOffset;
		dirOffset += (files[i].GetLength() + 1);
		fileInfoList[i] = files[i].GetFileInfo();
		hashList[i] = files[i].GetHash();
	}
}

// --------------------------------------------------------------------------

void BSAFile::PackFiles() {
	unsigned i;
	unsigned size;
	unsigned max;
	FILE *src, *tgt;
	char *aBuffer;

	max = header.hashOffset + header.number*sizeof(HashRecord);
	for(i = 0; i < header.number; i++) {
		if (max < files[i].GetSize())
			max = files[i].GetSize();
	}

	printf("Allocating %u bytes for the buffer\n", max);
	aBuffer = new char[max];

	if((tgt = fopen(name, "wb")) == NULL) {
		printf("FATAL: Cannot open file %s\n", name);
		exit(1);
	}

	prepareTables(aBuffer);

	fwrite(&header, sizeof(BSAHeader), 1, tgt);
	fwrite(aBuffer, sizeof(char), header.hashOffset + header.number*sizeof(HashRecord), tgt);
	
	qsort(files, header.number, sizeof(BSARecord), BSARecord::CompareByName);

	for(i = 0; i < header.number; i++) {
		size = files[i].GetSize();
		printf("Adding %s  (%u bytes)\n", files[i].GetName(), size);
		
		if((src = fopen(files[i].GetName(), "rb")) == NULL) {
			printf("ERROR: Can not open file %s\n", files[i].GetName());
		}
		if(fread(aBuffer, sizeof(char), size, src) != size) {
			printf("ERROR: error during reading %s\n", files[i].GetName());
			exit(1);
		}
		if(fwrite(aBuffer, sizeof(char), size, tgt) != size) {
			printf("ERROR: error during adding %s\n", files[i].GetName());
			exit(1);
		}
		fclose(src);
	}
	fclose(tgt);
	delete[] aBuffer;
}

// --------------------------------------------------------------------------

void packFiles(char *pName) {
	unsigned i;
	unsigned num;
	char **fileList;
	
	BSAFile *archive;

	printf("Looking for files...\n");
	fileList = new (char *[MAX_FILE_NUM]);
	num = scanFiles(fileList, "", 0);
	printf("Found %u files\n", num);
	
	archive = new BSAFile(pName, fileList, num);
	archive->PackFiles();
	delete archive;

	for(i = 0; i < num; i++) {
		delete[] fileList[i];
	}
	delete[] fileList;

	printf("Validating created archive...\n");
	archive = new BSAFile(pName);
	delete archive;
	printf("Archive %s has been created\n", pName);
}

// --------------------------------------------------------------------------

void help() {
	printf("\nUsage: BSAPACK (list|pack|unpack) bsa_file\n\n");
	printf("list   - list files in the archive\n\n");
	printf("pack   - pack ALL files in the subdirectories of the current directory into the\n");
	printf("         new BSA archive. Please, note, that files in the current directory itself\n");
	printf("         will not be packed - only files in all subdirectories (and their subdirs) will be\n\n");
	printf("unpack - unpack ALL files from the archive into the CURRENT directory\n");
	printf("         It is recommended to make sure that current directory is empty\n\n");
	exit(0);
}

// --------------------------------------------------------------------------

void main (int argc, char **argv) {
	BSAFile *archive;

	printf("\n TES3 BSA archiver v 0.1a                                    by ghostwheel\n\n");

	if (argc != 3)
		help();
	switch (argv[1][0]) {
		case 'l':
		case 'L':
			archive = new BSAFile(argv[2]);
			archive->PrintContent();
			delete archive;
			break;
		case 'p':
		case 'P':
			packFiles(argv[2]);
			break;
		case 'u':
		case 'U':
			archive = new BSAFile(argv[2]);
			archive->UnpackFiles();
			delete archive;
			break;
		default:
			help();
	}
}