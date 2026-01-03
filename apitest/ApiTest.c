#include <PalmOS.h>
#include <NetMgr.h>

/* Using HTTP instead of HTTPS because NetLib does not support SSL natively */
#define API_HOST "api.nextbillion.io"
#define API_PATH "/revgeocode?key=<YOUR_API_KEY>&at=40.1177,-83.1265"
#define HTTP_PORT 80

/* ---------------------------------------------------------------------
 * Helper to display text on the screen
 * --------------------------------------------------------------------- */
void DisplayText(const char* text, int x, int y) {
    WinDrawChars(text, StrLen(text), x, y);
}

/* ---------------------------------------------------------------------
 * Perform the Network API call using low-level NetLib (TCP/IP)
 * --------------------------------------------------------------------- */
void PerformApiCall() {
    UInt16 libRefNum;
    Err error;
    NetSocketRef sock = -1;
    NetHostInfoBufType hostBuf;
    NetHostInfoPtr hostInfo;
    NetSocketAddrINType addr;
    char buffer[256];
    char request[256];
    Int16 bytesRead;
    int yPos = 30;
    UInt16 ifErr;
    int line, i, start;
    char errBuf[64];

    DisplayText("Initializing NetLib...", 10, yPos);
    yPos += 12;

    /* Find the basic Network Library (Net.lib) */
    error = SysLibFind("Net.lib", &libRefNum);
    if (error != errNone) {
        DisplayText("Error: NetLib not found", 10, yPos);
        return;
    }

    /* Open NetLib */
    error = NetLibOpen(libRefNum, &ifErr);
    if (error != errNone && error != netErrAlreadyOpen) {
        StrPrintF(errBuf, "Error: NetLibOpen %d", error);
        DisplayText(errBuf, 10, yPos);
        return;
    }

    DisplayText("Resolving host...", 10, yPos);
    yPos += 12;

    /* Resolve hostname */
    hostInfo = NetLibGetHostByName(libRefNum, API_HOST, &hostBuf, SysTicksPerSecond() * 10, &error);
    if (!hostInfo) {
        StrPrintF(errBuf, "Error: DNS fail %d", error);
        DisplayText(errBuf, 10, yPos);
        goto cleanup;
    }

    /* Setup address structure */
    MemSet(&addr, sizeof(addr), 0);
    addr.family = netSocketAddrINET;
    addr.port = NetHToNS(HTTP_PORT);
    addr.addr = *(NetIPAddr*)hostInfo->addrListP[0];

    DisplayText("Connecting...", 10, yPos);
    yPos += 12;

    /* Open socket */
    sock = NetLibSocketOpen(libRefNum, netSocketAddrINET, netSocketTypeStream, 0, SysTicksPerSecond() * 10, &error);
    if (sock < 0) {
        StrPrintF(errBuf, "Error: SockOpen %d", error);
        DisplayText(errBuf, 10, yPos);
        goto cleanup;
    }

    /* Connect socket */
    if (NetLibSocketConnect(libRefNum, sock, (NetSocketAddrType*)&addr, sizeof(addr), SysTicksPerSecond() * 15, &error) < 0) {
        StrPrintF(errBuf, "Error: Connect %d", error);
        DisplayText(errBuf, 10, yPos);
        goto cleanup;
    }

    DisplayText("Sending request...", 10, yPos);
    yPos += 12;

    /* Construct and send HTTP GET request */
    StrPrintF(request, "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", API_PATH, API_HOST);
    NetLibSend(libRefNum, sock, request, StrLen(request), 0, NULL, 0, SysTicksPerSecond() * 5, &error);

    DisplayText("Reading response...", 10, yPos);
    yPos += 12;

    /* Read first chunk of response */
    MemSet(buffer, sizeof(buffer), 0);
    bytesRead = NetLibReceive(libRefNum, sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL, SysTicksPerSecond() * 10, &error);

    if (bytesRead > 0) {
        WinEraseWindow();
        WinDrawChars("HTTP Response:", 14, 10, 10);
        
        /* Simple text wrapping for display */
        line = 25;
        start = 0;
        for (i = 0; i < bytesRead && line < 150; i++) {
            if (i > 0 && i % 25 == 0) {
                WinDrawChars(buffer + start, i - start, 10, line);
                line += 10;
                start = i;
            }
        }
        if (start < bytesRead && line < 150) {
            WinDrawChars(buffer + start, bytesRead - start, 10, line);
        }
    } else {
        StrPrintF(errBuf, "Error: Recv %d", error);
        DisplayText(errBuf, 10, yPos);
    }

cleanup:
    if (sock >= 0) {
        NetLibSocketClose(libRefNum, sock, SysTicksPerSecond() * 5, &error);
    }
}

/* ---------------------------------------------------------------------
 * PilotMain is called by the startup code and implements a simple event
 * handling loop.
 * --------------------------------------------------------------------- */
UInt32 PilotMain( UInt16 cmd, void *cmdPBP, UInt16 launchFlags )
{
    EventType event;

    if (cmd == sysAppLaunchCmdNormalLaunch) {

        /* Initial display */
        WinEraseWindow();
        WinDrawChars( "Palm OS API Test", 16, 35, 10 );

        /* Perform the API call */
        PerformApiCall();

        /* Main event loop: */
        do {
            /* Doze until an event arrives. */
            EvtGetEvent( &event, evtWaitForever );

            /* System gets first chance to handle the event. */
            SysHandleEvent( &event );

        /* Return from PilotMain when an appStopEvent is received. */
        } while (event.eType != appStopEvent);
    }
    return 0;
}
