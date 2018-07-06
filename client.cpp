#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <sstream>
#include <opencv2/opencv.hpp>
#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace cv;
int main()
{
	string ipAddress = "127.0.0.1";			// IP Address of the server
	int port = 8000;						// Listening port # on the server

											// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return 0;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 0;
	}

	// Do-while loop to send and receive data
	char revData[1000000] = "";
	IplImage *image_src = cvCreateImage(cvSize(640, 480), 8, 1);// create gray
	
	int i, j;
	int ret;
	cvNamedWindow("client", 1);

	CvVideoWriter *pW = cvCreateVideoWriter("D:\\output.avi", CV_FOURCC('M','S','V','C'), 15, cvSize(image_src->width, image_src->height), false);
	
	while (true)
	{
		//receive gray image
		ret = recv(sock, revData, 1000000, 0);
		if (ret > 0)
		{
			//revData[ret] = 0x00;
			for (i = 0; i < image_src->height; i++)
			{
				for (j = 0; j < image_src->width; j++)
				{
					((char *)(image_src->imageData + i * image_src->widthStep))[j] = revData[image_src->width * i + j];
				}
			}
			ret = 0;
		}
		cvShowImage("client", image_src);
		
		cvWriteFrame(pW, image_src);
			
		char c = (char)cvWaitKey(33);
		if ( c == 27)
			break;
	}
	cvDestroyWindow("client");
	
	cvReleaseImage(&image_src);
	cvReleaseVideoWriter(&pW);

	// Gracefully close down everything
	closesocket(sock);
	WSACleanup();
	return 0;
}