/****************************************************************************
**
** Copyright 2015 by Emotiv. All rights reserved
** Example - AverageBandPowers
** The average band power for a specific channel from the latest epoch with
** 0.5 seconds step size and 2 seconds window size
** This example is used for single connection.
**
****************************************************************************/
#ifdef __cplusplus


#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#endif
#if __linux__ || __APPLE__
    #include <unistd.h>
#endif

#include <iostream>
#include <string>
#include <fstream>

#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

#define ADDRESS "127.0.0.1"
#define PORT 7771

#define OUTPUT_BUFFER_SIZE 1024

extern "C"
{
#endif

#include "IEmoStateDLL.h"
#include "Iedk.h"
#include "IedkErrorCode.h"
#include "IEmoStatePerformanceMetric.h"

using namespace std;

#if __linux__ || __APPLE__
int _kbhit(void);
#endif
#ifdef __APPLE__
int _kbhit (void)
{
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}
#endif

int  main()
{

	EmoEngineEventHandle eEvent	= IEE_EmoEngineEventCreate();
	EmoStateHandle eState       = IEE_EmoStateCreate();

	UdpTransmitSocket transmitSocket( IpEndpointName( ADDRESS, PORT ) );    
	char buffer[OUTPUT_BUFFER_SIZE];

	unsigned int userID   = 0;
	bool ready = false;
	int state  = 0;

	IEE_DataChannel_t channelList[] = { IED_AF3, IED_AF4 }; // IED_AF4, IED_T7, IED_T8, IED_Pz
#ifndef __APPLE__
	std::string ouputFile = "AverageBandPowers.txt";
#else
    std::string home_path;
    const char* home = getenv("HOME");
    home_path.assign(home);
    home_path.append("/Desktop/AverageBandPowers.csv");
    std::string ouputFile = home_path;
#endif
	const char header[] = "Theta, Alpha, Low_beta, High_beta, Gamma";
    std::ofstream ofs(ouputFile.c_str(), std::ios::trunc);
	ofs << header << std::endl;
		
	if (IEE_EngineConnect() != EDK_OK) {
        std::cout << "Emotiv Driver start up failed.";
       return -1;
	}

	std::cout << "==================================================================="
		      << std::endl;
    std::cout << "  Example to get the average band power for a specific channel from \n"
		         "the latest epoch "
		      << std::endl;
    std::cout << "==================================================================="
		      << std::endl;

	while (!_kbhit())
	{
		state = IEE_EngineGetNextEvent(eEvent);

		if (state == EDK_OK) 
		{
		    IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
		    IEE_EmoEngineEventGetUserId(eEvent, &userID);

			if (eventType == IEE_UserAdded) {
		         std::cout << "User added" << std::endl;
				 IEE_FFTSetWindowingType(userID, IEE_HAMMING);

				 std::cout << header << std::endl;
		         ready = true;
			}
		}

		if (ready)
		{
            double alpha, low_beta, high_beta, gamma, theta;
			double alphaB, low_betaB, high_betaB, gammaB, thetaB;
			IEE_EEG_ContactQuality_t cq;
            alpha = low_beta = high_beta = gamma = theta = 0;

//            for(int i=0 ; i< sizeof(channelList)/sizeof(channelList[0]) ; ++i)
//            {
			int i = 0;
                int result = IEE_GetAverageBandPowers(userID, channelList[i], &theta, &alpha, 
					                                     &low_beta, &high_beta, &gamma);
				result = IEE_GetAverageBandPowers(userID, channelList[1], &thetaB, &alphaB, 
					                                     &low_betaB, &high_betaB, &gammaB);
                if(result == EDK_OK){

					cq = IS_GetContactQuality(eState, (IEE_InputChannels_t)channelList[i]);
					int cqInt = 0;			

					std::cout << "Contact quality: ";
					switch (cq) {
						case IEEG_CQ_NO_SIGNAL : {cqInt = 0; cout << "NO SIGNAL"; break;}
						case IEEG_CQ_VERY_BAD  : {cqInt = 1; cout << "VERY BAD"; break;}
						case IEEG_CQ_POOR : {cqInt = 2; cout << "POOR"; break;}
						case IEEG_CQ_FAIR : {cqInt = 3; cout << "FAIR"; break;}
						case IEEG_CQ_GOOD : {cqInt = 4; cout << "GOOD"; break;}
					}
                    std::cout << " " << endl;

					IEE_SignalStrength_t ws = IS_GetWirelessSignalStatus(eState);
					int wsInt = 0;

					std::cout << "\nWireless signal: ";
					switch (ws) {
						case NO_SIG : {wsInt = 0; cout << "NO SIGNAL"; break;}
						case BAD_SIG  : {wsInt = 1; cout << "BAD"; break;}
						case GOOD_SIG : {wsInt = 2; cout << "GOOD"; break;}
					}
                    std::cout << std::endl << endl;

					int emo = IS_PerformanceMetricIsActive(eState, PM_EXCITEMENT);

					float excL = IS_PerformanceMetricGetExcitementLongTermScore(eState);
					float exc = IS_PerformanceMetricGetInstantaneousExcitementScore(eState);
					float rel =	IS_PerformanceMetricGetRelaxationScore(eState);
					float str =	IS_PerformanceMetricGetStressScore(eState);
					float eng =	IS_PerformanceMetricGetEngagementBoredomScore(eState);
					float itr =	IS_PerformanceMetricGetInterestScore(eState);
					float foc = IS_PerformanceMetricGetFocusScore(eState);

					std::cout << theta << "," << alpha << "," << low_beta << ",";
                    std::cout << high_beta << "," << gamma << std::endl;

					cout<<"excitation LT "<<excL<<", exc "<<exc<<", relax "<<rel<<", stress "<<str
						<<", engage/bored "<<eng<<", interest "<<itr<<", focus"<<foc<<endl;

					osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );    
					p << osc::BeginBundleImmediate
						<< osc::BeginMessage( "/contactSignal" ) 
							<< cqInt << osc::EndMessage
						<< osc::BeginMessage( "/wirelessSignal" ) 
							<< wsInt << osc::EndMessage
						<< osc::BeginMessage( "/eegtheta" ) 
							<< (float)theta << osc::EndMessage
						<< osc::BeginMessage( "/eeglowalpha" ) 
							<< (float)alpha << osc::EndMessage
						<< osc::BeginMessage( "/eeglowbeta" ) 
							<< (float)low_beta << osc::EndMessage
						<< osc::BeginMessage( "/eeghighbeta" ) 
							<< (float)high_beta << osc::EndMessage
						<< osc::BeginMessage( "/eegmidgamma" ) 
							<< (float)gamma << osc::EndMessage
						<< osc::BeginMessage( "/emoOnOff" ) 
							<< emo << osc::EndMessage
						<< osc::BeginMessage( "/excLongTerm" ) 
							<< (float)excL << osc::EndMessage
						<< osc::BeginMessage( "/excitation" ) 
							<< (float)exc << osc::EndMessage
						<< osc::BeginMessage( "/relaxation" ) 
							<< (float)rel << osc::EndMessage
						<< osc::BeginMessage( "/stress" ) 
							<< (float)str << osc::EndMessage
						<< osc::BeginMessage( "/engagementBoredom" ) 
							<< (float)eng << osc::EndMessage
						<< osc::BeginMessage( "/interest" ) 
							<< (float)itr << osc::EndMessage
						<< osc::BeginMessage( "/focus" ) 
							<< (float)foc << osc::EndMessage
						<< osc::EndBundle;
    
					transmitSocket.Send( p.Data(), p.Size() ); 
					
 //               } // end for

				// attention = delta (alpha)
				// imagination = avg (alpha)

				float att = alpha - alphaB;
				float img = (alpha + alphaB) / (low_beta + low_betaB);
				osc::OutboundPacketStream pB( buffer, OUTPUT_BUFFER_SIZE );  
				pB << osc::BeginBundleImmediate
					<< osc::BeginMessage( "/attention" ) 
						<< att << osc::EndMessage
					<< osc::BeginMessage( "/imagination" ) 
						<< img << osc::EndMessage
					<< osc::EndBundle;
				transmitSocket.Send( pB.Data(), pB.Size() ); 
            }
		}

#ifdef _WIN32
        Sleep(1);
#endif
#if linux || __APPLE__
        usleep(10000);
#endif
	}

	ofs.close();

	IEE_EngineDisconnect();
	IEE_EmoStateFree(eState);
	IEE_EmoEngineEventFree(eEvent);
    return 0;
}

#ifdef __linux__
int _kbhit(void)
{
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;

    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd,NULL, NULL, &tv) == -1)
    return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
}
#endif

#ifdef __cplusplus
}
#endif
