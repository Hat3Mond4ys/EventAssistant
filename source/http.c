#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "util.h"
#include "certs/cybertrust.h"
#include "certs/digicert.h"

Result http_download(PrintConsole topScreen, PrintConsole bottomScreen, httpcContext *context) {	
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	consoleSelect(&bottomScreen);
	
	ret = httpcAddRequestHeaderField(context, (char*)"User-Agent",  (char*)"ECI-TOOL");
	if (ret != 0)
		return ret;  
		
	ret = httpcSetSSLOpt(context, 1<<9);
	if (ret != 0)
		return ret; 
		
	httpcAddTrustedRootCA(context, cybertrust_cer, cybertrust_cer_len);
	httpcAddTrustedRootCA(context, digicert_cer, digicert_cer_len);
	
	ret = httpcBeginRequest(context);
	if (ret != 0)		
		return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if (ret != 0) {
		printf("\x1b[26;0HStatus: \x1b[31mNO INTERNET AVAILABLE\x1b[0m");
		return ret;
	}
	
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	if (statuscode == 200)
		printf("\x1b[26;0HStatus: \x1b[32mOKAY                  \x1b[0m");
	else 
		printf("\x1b[26;0HStatus: \x1b[31mFILE NOT AVAILABLE YET\x1b[0m");
	
	gfxFlushBuffers();
	gfxSwapBuffers();

	if (statuscode != 200) {
		if (statuscode >= 300 && statuscode < 400) {
			char newUrl[1024];
			httpcGetResponseHeader(context, (char*)"Location", newUrl, 1024);
			httpcCloseContext(context);
			ret = http_download(topScreen, bottomScreen, context);
			return ret;
		}
		return -2;
	}

	ret = httpcGetDownloadSizeState(context, NULL, &contentsize);
	if (ret != 0)
		return ret;
	
	printf("\x1b[27;0HDownload size: %d bytes    ", (int)contentsize);

	gfxFlushBuffers();
	gfxSwapBuffers();

	buf = (u8*)malloc(contentsize);
	if (buf == NULL)
		return -1;
	memset(buf, 0, contentsize);

	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret != 0) {
		free(buf);
		return ret;
	}

	consoleSelect(&topScreen);
	printf("%s", buf);
	free(buf);
	return 0;
}

void getText(PrintConsole topScreen, PrintConsole bottomScreen, char *url) {
	Result ret = 0;
	httpcContext context;
	httpcInit(0);
	consoleSelect(&bottomScreen);	
	gfxFlushBuffers();

	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	
	if (ret != 0) 
		printf("\x1b[25;0HStatus: \x1b[31mURL NOT AVAILABLE\x1b[0m");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	if (ret == 0) {
		printf("\x1b[25;0HDownloading...");
		ret = http_download(topScreen, bottomScreen, &context);
		gfxFlushBuffers();
		httpcCloseContext(&context);
	}
	httpcExit();
}

void printDistro(PrintConsole topScreen, PrintConsole bottomScreen, char *url) {
	consoleSelect(&bottomScreen);
	printf("\x1b[2J");
	printf("----------------------------------------");
	printf("NA  - North America");
	printf("\nPAL - Europe, Australia");
	printf("\nJPN - Japan");
	printf("\nKOR - South Korea");
	printf("\nALL - All regions available\n");
	printf("----------------------------------------");
	printf("\x1b[29;10HPress A to continue.");
	consoleSelect(&topScreen);		
	printf("\x1b[2J");
	getText(topScreen, bottomScreen, url);
	
	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_A) 
			break; 			 
	}
}

void printPSdates(PrintConsole topScreen, PrintConsole bottomScreen, char *url, int page) {
	consoleSelect (&bottomScreen);
	printf("\x1b[5;0HPage %d/7 ", page);
	consoleSelect(&topScreen);		
	printf("\x1b[2J");
	getText(topScreen, bottomScreen, url);
	
	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_A) 
			break; 			 
	}
}

Result downloadFile(PrintConsole topScreen, PrintConsole bottomScreen, char* url, char* path) {
	fsInit();
	httpcInit(0);
	
	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	printf("Downloading file from: \x1b[32m%s\x1b[0m to: \x1b[32m%s\x1b[0m\n", url, path);
		
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	if (ret != 0) {
		printf("Error in: httpcOpenContext. Return: %lx\n", ret);
		return ret;
	}
	
	ret = httpcAddRequestHeaderField(&context, "User-Agent", "ECI-TOOL");
	if (ret != 0) {
		printf("\x1b[31mError in: httpcAddRequestHeaderField.\x1b[0m Return: %lx\n", ret);
		return ret;
	}
	
	ret = httpcSetSSLOpt(&context, 1<<9);
	if (ret != 0) {
		printf("\x1b[31mError in: httpcSetSSLOpt.\x1b[0m Return: %lx\n", ret);
		return ret;
	}
	
	httpcAddTrustedRootCA(&context, cybertrust_cer, cybertrust_cer_len);
	httpcAddTrustedRootCA(&context, digicert_cer, digicert_cer_len);
	
	ret = httpcBeginRequest(&context);
	if(ret != 0) {
		printf("\x1b[31mError in: httpcBeginRequest.\x1b[0m Return: %lx\n", ret);
		return ret;
	}
	
	ret = httpcGetResponseStatusCode(&context, &statuscode, 0);
	if (ret != 0) {
		printf("\x1b[31mError in: httpcGetResponseStatusCode.\x1b[0m Return: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	if (statuscode != 200) {
		if (statuscode >= 300 && statuscode < 400) {
			char newUrl[1024];
			ret = httpcGetResponseHeader(&context, (char*)"Location", newUrl, 1024);
			if (ret != 0) {
				printf("\x1b[31mCould not get relocation header in 3XX http response.\x1b[0m\n");
				return ret;
			}
			httpcCloseContext(&context);
			printf("Retrying to call download function...\n\n");
			ret = downloadFile(topScreen, bottomScreen, newUrl, path);
			return ret;
		}
		else {
			printf("\x1b[31mError: status code not 200 or redirection (3XX).\x1b[0m\nStatus code: %lu\n", statuscode);
			httpcCloseContext(&context);
			return -1;
		}
	}
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0) {
		printf("\x1b[31mError in: httpcGetDownloadSizeState.\x1b[0m Return: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	buf = (u8*)malloc(contentsize);
	if (buf == NULL) {
		printf("\x1b[31mFailure to malloc buffer.\x1b[0m\n");
		return -2;
	}
	memset(buf, 0, contentsize);
	
	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret != 0) {
		free(buf);
		printf("\x1b[31mError in: httpcDownloadData.\x1b[0m Return: %lx\n", ret);
		httpcCloseContext(&context);
		return ret;
	}
	
	remove(path);
	FILE *fptr = fopen(path, "wb");
	fwrite(buf, 1, contentsize, fptr);
	fclose(fptr);
	free(buf);
	
	httpcCloseContext(&context);
	httpcExit();
	fsExit();
	
	return 0;
}