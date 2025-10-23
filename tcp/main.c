#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint64_t SystemMonotonicMS()
{
	long            ms;
	time_t          s;

	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	s  = spec.tv_sec;
	ms = (spec.tv_nsec / 1000000);

	uint64_t result = s;
	result *= 1000;
	result += ms;

	return result;
}


#ifdef TCPSERVER
	#include "libs/TCPServer.h"

	int main()
	{
		TCPServer server;
		
		TCPServer_Initiate(&server, "8080");

		while(1)
		{
			TCPServer_Work(&server);
		}

		TCPServer_Dispose(&server);

		return 0;
	}

#else //TCPCLIENT

	#include "libs/TCPClient.h"
	
	int main()
	{
		TCPClient client;
		/*uint8_t buffer[1024];*/
		const char* message = "Fog everywhere. Fog up the river, where it flows among green aits and meadows; fog down the river, where it rolls deified among the tiers of shipping and the waterside pollutions of a great (and dirty) city. Fog on the Essex marshes, fog on the Kentish heights. Fog creeping into the cabooses of collier-brigs; fog lying out on the yards and hovering in the rigging of great ships; fog drooping on the gunwales of barges and small boats. Fog in the eyes and throats of ancient Greenwich pensioners, wheezing by the firesides of their wards; fog in the stem and bowl of the afternoon pipe of the wrathful skipper, down in his close cabin; fog cruelly pinching the toes and fingers of his shivering little apprentice boy on deck. Chance people on the bridges peeping over the parapets into a nether sky of fog, with fog all round them, as if they were up in a balloon and hanging in the misty clouds.\r\n";
		int length = strlen(message) + 1;

		TCPClient_Initiate(&client);

		if (TCPClient_Connect(&client, "localhost", "8080") != 0)
		{
			printf("Kunde inte ansluta till servern\r\n");
			return -1;
		}

		const char* ptr = &message[0];
		int bytesLeft = length;

		uint64_t now = SystemMonotonicMS();
		uint64_t timeout = now + 5000;
		
		while(bytesLeft > 0 && now < timeout)
		{
			now = SystemMonotonicMS();
			
			int bytesSent = TCPClient_Write(&client, (uint8_t*)ptr, bytesLeft);
			if(bytesSent > 0)
			{
				ptr += bytesSent;
				bytesLeft -= bytesSent;
			}
		}
		if(bytesLeft > 0)
		{
			printf("TIMEOUT ON WRITE!\r\n");
			return 1;
		}

		uint8_t inbuffer[1024];
		memset(inbuffer, 0, sizeof(inbuffer));
		int totalBytesRead = 0;

		now = SystemMonotonicMS();
		timeout = now + 5000;


		while(now < timeout)
		{
			now = SystemMonotonicMS();

			int bytesRead = TCPClient_Read(&client, &inbuffer[totalBytesRead], sizeof(inbuffer) - totalBytesRead);

			if (bytesRead > 0){
				totalBytesRead += bytesRead;
			}
			
			printf("bytesRead: %d\nTotalBytesRead: %d \n", bytesRead, totalBytesRead);
			

			if(totalBytesRead == length)
				break;
		}

		if(totalBytesRead != length){
			printf("TIMEOUT ON READ!\n");
		}else{
			printf("%s", inbuffer);
		}


		
		TCPClient_Dispose(&client);

		return 0;

	}

#endif