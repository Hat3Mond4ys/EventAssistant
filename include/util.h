#include <stdlib.h>
#include <3ds.h>

void refresh(int currentEntry, PrintConsole topScreen, char *lista[], int N);
void refreshDB(int currentEntry, PrintConsole topScreen, char *lista[], int N, int page);
void update(PrintConsole topScreen, PrintConsole bottomScreen);
u32 CHKOffset(u32 i, int game);
u32 CHKLength(u32 i, int game);
u16 ccitt16(u8* data, u32 len);
void rewriteCHK(u8 *mainbuf, int game);
void credits(PrintConsole topScreen, PrintConsole bottomScreen);
bool isHBL();
void fsStart();
void fsEnd();
bool openSaveArch(FS_Archive *out, u64 id);
void errDisp(PrintConsole topScreen, int i);
void infoDisp(PrintConsole screen, int i);
