struct CryptTable {
	unsigned int** tables;
	unsigned short* additionalTables;

	unsigned int startValue;
};

struct CryptStatus {
	unsigned int currentValue;
	unsigned int additionalValuePerStep;
};

void GenerateCryptTables(CryptTable* &CryptTables, unsigned ModValue);
void GenerateLoginServerCryptTables(CryptTable* &CryptTables);
void FreeCryptTables(CryptTable* &CryptTables);
int  DecryptBufferHeader(CryptStatus *ri, CryptTable *CryptTables, unsigned char *Buffer);
bool DecryptBufferData(CryptTable *CryptTables, unsigned char *Buffer);
void EncryptBuffer(CryptTable *CryptTables, unsigned char *Buffer);